# CODE_REVIEW — S-REVIEW (planning-protocol v3.6 section 3.8)

Reviewer: fresh-context reviewer, did not author the code. Branch `impl-run-20261027T120000Z-guitar-amp-vst`.
Method: read-only review of `Source/`, `tests/`, `tools/`, `.github/workflows/`, `CMakeLists.txt`, `Resources/` (data only), plus the plan documents.
No build toolchain and no JUCE checkout were available. Behaviour that depends on JUCE internals is marked **unverified**.
Two numeric claims were checked with small Python scripts (CR-003). Dependency versions were compared with `gh api` on 2026-10-31.
Frozen tests are not criticised for being frozen. Where a test is mentioned, the point is a coverage gap.

## Summary

| Severity | Count | IDs |
|---|---|---|
| Critical | 2 | CR-001, CR-002 |
| High | 6 | CR-003 to CR-008 |
| Medium | 11 | CR-009 to CR-019 |
| Low | 9 | CR-020 to CR-028 |
| **Total** | **28** | |

Rubric: Critical means silently wrong results or a verification that reports success without running. High means a defect that is reachable in normal use or a thread/memory-safety defect. Medium and Low are weighted by reach and fix cost.

Known item from the brief:
- Confirmed: `winLen` is unused and the window is a 4096-sample box. See CR-009.
- Refuted as a functional bug: `reset()` not clearing `fifo`/`fifoPos` is harmless. See CR-021.
- A different defect in the same file turned out to be more serious. See CR-003.

---

## Critical

### CR-001 — Three CI security gates can never fail (negated `grep` followed by `echo`)
- **Severity:** Critical (verification that reports success without running)
- **Where:** `.github/workflows/ci.yml:78` (T-041), `:90` (T-040 static), `:108` (T-040 Windows import table)
- **What is wrong:**
  - Each step ends with `! grep ...` (or `! grep ... | grep -v ... | grep -v ...`) and then `echo "... pass"`.
  - GitHub runs `shell: bash` as `bash -eo pipefail`, but bash's `errexit` ignores a pipeline whose status is inverted with `!`.
  - A match therefore does not stop the script, `echo` returns 0, and the step is green.
  - I reproduced this with `bash --noprofile --norc -eo pipefail -c '! echo offender | grep -v zzz | grep -v yyy; echo pass'`. It printed the offender, then `pass`, and exited 0.
  - `tools/ci/lint_ci.py` (Rule 14 linter) only checks `|| true`, `continue-on-error`, `set +e` and `| tee`. It does not detect this idiom, so the lint job passes too.
  - This is the same class of defect the project already fixed once (D-001p, "check that could not fail"), reintroduced in a new form.
- **Failure scenario:** Someone adds `juce::URL` or a `ws2_32` import, or file I/O outside the allowed modules. T-040 and T-041 stay green and the "no network / local files only" claims in README and the plan are unenforced.
- **Fix:** Use an explicit test: `if grep -q ...; then echo "FAIL"; exit 1; fi`. For the pipeline: `offenders=$(grep ... | grep -v ... || true); [ -z "$offenders" ] || { echo "$offenders"; exit 1; }`. Add a `lint_ci.py` rule that rejects a line starting with `! ` inside a `run:` block unless it is the last command of the step. Add a selftest case: a step containing `! grep -q x f; echo ok` must be flagged.

### CR-002 — `setStateInformation` desynchronises the DSP from the parameters (stale voicing, cab and oversampling) and races the audio thread
- **Severity:** Critical (silently wrong audio after session recall)
- **Where:** `Source/PluginProcessor.cpp:491-509` (writes at `:500-502`), consumers `:230-285` (`applySteppedParams`)
- **What is wrong:**
  - After `apvts.replaceState`, the message thread sets `currentVoicing = pendingVoicing`, `currentCab = pendingCab` and `currentOs = pendingOs` to the restored values, and recomputes `loudnessGain`.
  - On the next block `applySteppedParams` reads the parameters, sees `pending == current`, and takes no branch.
  - So `preamp.setVoicing`, `toneStack.setVoicing`, `powerAmp.setVoicing`, `cabIR.selectCab` and `osStage.prepare` never run.
  - The DSP keeps the old voicing, cab and oversampling mode. Only the loudness trim is switched to the new voicing.
  - This happens whenever the host calls `setStateInformation` after `prepareToPlay`.
  - These are plain `int`/`float` members written by the message thread while the audio thread reads and writes them. That is a data race and undefined behaviour. The `osStage` object is not touched here, but the mirrors that decide whether to re-prepare it are.
  - T-009 does not catch this. It builds a fresh, never-prepared processor and compares parameter values only. No test checks the DSP state after a restore.
- **Failure scenario:**
  - Plugin loaded with Tweed and "British 4x12". The host recalls a project saved with 5150, "Doom 4x15" and Eco.
  - The UI shows the recalled settings but the audio is still Tweed, the old cab and High (4x) oversampling.
  - The reported latency is stale. It stays wrong until the user touches those three controls.
- **Fix:**
  - Do not touch the mirrors in `setStateInformation`. Let `applySteppedParams` detect the change on the audio thread.
  - If the mirrors must be initialised (state restored before prepare), do it in `prepareToPlay` from the parameters.
  - Add a test: prepare, then `setStateInformation` with a different voicing, cab and OS, process a block, and assert the render equals a processor constructed with that state.

---

## High

### CR-003 — Noise-gate RMS accumulator can go negative in digital silence, which latches `envelope = NaN` and disables the gate permanently
- **Severity:** High (could be argued Critical: silent loss of a feature; limited audible blast radius)
- **Where:** `Source/DSP/NoiseGate.h:59-72` (also `:25-32` `reset`)
- **What is wrong:**
  - `runningSumOfSquares` is a running double sum with add and subtract. After loud material followed by at least 4096 samples of exact zeros, the true sum is 0 but the stored value is a rounding residual of about ±1e-13, and its sign is random.
  - I emulated the exact arithmetic in Python (float32 squares, double accumulator, 4096 FIFO). In 3 of 5 seeds the residual was negative (about -2e-13 to -3e-13) and stayed negative for every remaining silent sample.
  - `std::sqrt((float)(negative/4096))` is NaN. Then `envelope += coef*(rms-envelope)` is NaN forever, because a NaN `rms` is never `> envelope` and NaN propagates.
  - `gainToDecibels(NaN, -120)` returns -120, and `envelope < thrLinear` is false for NaN, so `target` is always 1.0. The gate then passes everything.
  - The state is only cleared by `prepare()`. `AudioProcessor::reset()` is not overridden (CR-012).
- **Failure scenario:** A DAW track plays a riff, then has an empty gap of 4096 or more digital-zero samples (a muted region, a cut clip, a transport gap). The gate silently stops working for the rest of the session, and hiss and noise between notes return. No test fails.
- **Fix:**
  - Clamp: `std::sqrt(std::max(0.0, sum / n))`.
  - Better, drop the running sum. Keep a one-pole mean-square follower, or recompute the sum exactly per block. If the sum is kept, re-zero it when `rmsWindowFill == fifoSize` and the block is silent, or periodically rebuild it from the FIFO.
  - Guard the envelope: `if (! std::isfinite(envelope)) envelope = 0;`.
  - Add a test with 0.3 s loud noise followed by at least 0.2 s of exact zeros, asserting the envelope stays finite.

### CR-004 — Single filter, envelope and gate state is shared by both channels and run sequentially
- **Severity:** High (audible block-rate discontinuities in stereo, L and R differ)
- **Where:**
  - Gate: `Source/PluginProcessor.cpp:330-332` (same `NoiseGate` instance called for ch0 then ch1; state at `NoiseGate.h:99-108`)
  - Preamp: `PluginProcessor.cpp:369` with `Source/DSP/Preamp.h:95-157` (`juce::dsp::IIR::Filter<float> preEmphasis, postEmphasis` are single-channel filters)
  - Power amp: `PluginProcessor.cpp:372`, `PowerAmp.h:291` (`env` shared)
  - Contrast: `Boost`, `ToneStack`, `PostEq` use `ProcessorDuplicator` (per-channel state), and oversampling is per-channel.
- **What is wrong:**
  - Per block, channel 0 runs through the shared state and channel 1 then continues from the state channel 0 left.
  - Channel 0 of the next block starts from channel 1's end state. Even for identical L and R input, each channel sees a state jump at every block boundary, and L differs from R.
  - For different L and R content, the gate detector, the preamp biquad memory and the sag envelope are driven by the other channel's audio.
- **Failure scenario:** A stereo guitar track processed at 512-sample blocks. The nonlinear stages amplify the boundary mismatch into a buzz at the block rate (about 94 Hz at 48 kHz). For a track with different L and R (stereo DI), the gate on R opens or closes based on L.
- **Fix:** Instantiate per-channel state: `std::array<PreampStage,2>`, `std::array<NoiseGate,2>`, per-channel `env`. Or use `ProcessorDuplicator` for the biquads and hold `env[ch]`. Add a test with different L and R signals, asserting each output channel is bit-identical to a mono run of that channel alone.

### CR-005 — Real-time violations in `processBlock` (allocation, deallocation, host calls)
- **Severity:** High (the repo's own audit claims there are none)
- **Where and what:**
  - `ToneStack.h:50-61`, `PowerAmp.h:85`, `PostEq.h:30-33`: `*filter.state = *Coefficients::makeX(...)` allocates a new `Coefficients` object. This runs every block: 5 allocations in the tone stack, 1 for the thump shelf and 2 for post-EQ, unconditionally, even when the gains did not change.
  - `PluginProcessor.cpp:242-249`: an oversampling change runs `osStage.prepare`, which does `make_unique<Oversampling>`, `initProcessing` and destroys the old one. It also reallocates every filter via `preamp/toneStack/powerAmp.prepare`, all on the audio thread.
  - `PluginProcessor.cpp:249` and `:199`: `setLatencySamples` is called from the audio thread, which calls into the host (JUCE notifies the host; the effect on a given host is unverified).
  - `CabIR.h:74` copies the whole user IR twice (`AudioBuffer first(userIrBuffer), second(...)`) per swap. A 30 s, 96 kHz stereo IR is about 23 MB each.
  - `CabIR.h:122` frees the previous `userIrBuffer`.
  - `CabIR.h:206-232` builds a `juce::String` for up to 32 comparisons on each factory-cab switch.
  - `PluginProcessor.cpp:277` `takeLoadedBuffer` takes a `CriticalSection` on the audio thread (see CR-006).
  - Whether `Convolution::loadImpulseResponse(AudioBuffer&&)` itself allocates is **unverified**. The comment "wait-free" in `CabIR.h:65` and finding 5 of `validation/automation-audit.md` (the "no locks, no allocations on the audio path" line) rest on a JUCE doc reading that I could not check.
- **Failure scenario:** At 64-sample buffers, the per-block allocations, an OS toggle, or loading a long user IR produce dropouts. The allocator can also take a lock under another thread's contention.
- **Fix:**
  - Compute filter coefficients into a pre-allocated `Coefficients` object by assigning the fields in place. Or use `dsp::IIR::Filter` with `coefficients` swapped from a pre-built set. Or recompute only when a value changed beyond an epsilon.
  - Build the new `Oversampling` object on the message thread, then swap it in via an atomic pointer or pre-allocate both modes.
  - Do the `loadImpulseResponse` hand-off from the loader thread or message thread (it is documented thread-safe), not from `processBlock`. Preload both oversampling configurations.
  - Replace `getNamedBinaryData` with a constant table.
  - Correct the audit document.

### CR-006 — `UserIRLoader` holds `lock` across the entire load; superseding is impossible, a request can be lost, shutdown can kill the thread
- **Severity:** High
- **Where:** `Source/DSP/UserIRLoader.cpp:19-27` (`request`), `:35` (`take`), `:49` (`run` takes `ScopedLock sl(lock)` and holds it to the end of the function), `:99` (generation check), `:13-15` (destructor)
- **What is wrong:**
  - `run()` takes the lock at the start and keeps it through file open, read, resample and the hand-off.
  - `request()` takes the same lock. A second request from the message thread therefore blocks (UI freeze) for the full duration of the in-flight load, which can be seconds for a long file. It cannot "abandon" the running load as the header promises (`UserIRLoader.h:20`).
  - Because `run` reads `generation` while holding the lock that `request` needs to bump it, `gen == generation.load()` at `:99` is always true. The supersede check is dead code.
  - After `run` returns, the blocked `request` proceeds to `startThread()`. If the previous thread's handle has not been closed yet, `startThread()` is a no-op in JUCE. The new request is then dropped, `loading` stays true and `isBusy()` stays true forever. This is **unverified** because the JUCE source is not checked out. It is a classic race: the waiter wakes the moment `run` releases the lock.
  - `take()` (audio thread, `:35`) acquires the same `CriticalSection` after `ready.exchange(false)`. If `ready` is still true from a finished load when a new load starts, the audio thread blocks until that second load completes. That is a blocking lock in `processBlock`.
  - `~Impl` calls `stopThread(2000)`. `run()` never checks `threadShouldExit()`. A slow load (network drive or a large file) makes JUCE force-kill the thread while it holds the lock, an open reader and the `afm`, which is undefined behaviour and a leak. This is also a 2 s host hang on plugin unload.
- **Fix:**
  - Hold the lock only to copy the request and to publish the result.
  - Use a loop or `notify()` design with `wait()`, or a `ThreadPool` job per request with a cancellation flag checked between steps.
  - Publish the finished buffer through an atomically swapped `shared_ptr` or a lock-free single-slot exchange, so the audio thread never takes a mutex.
  - Check `threadShouldExit()` between read and resample, and in the destructor set the cancel flag before `stopThread`.

### CR-007 — User-IR file validation: integer truncation, unbounded resample size, unchecked read result, NaN or hot content
- **Severity:** High (host crash or out-of-memory by a crafted or unusual file; NaN or very loud output)
- **Where:** `Source/DSP/UserIRLoader.cpp:65-87`; `Source/DSP/CabIR.h:75-81` (`Normalise::no`)
- **What is wrong:**
  - `:65` `(int) reader->lengthInSamples` truncates an `int64`. A WAV header declaring 2^32+100 samples becomes 100, so a silently truncated IR loads. A value of 2^31 or more can be negative and is rejected, which is fine, but the truncation case is not.
  - The `maxLen` cap (`:66`) is applied to source samples. The resampled length is `dstLen = ceil(len * rate / srcRate)` (`:83`) with no cap. A 8 kHz file with 2.88 M samples at a 192 kHz host gives 69 M samples, so about 550 MB for the stereo buffer plus the convolution engine partitions.
  - A very low declared rate (for example 1 Hz) makes `(int) ceil(...)` overflow, which is undefined behaviour.
  - `:72` `reader->read(...)` return value is ignored. On a read failure or short read of a corrupt file, part of `raw` may be uninitialised (`AudioBuffer(2, len)` does not clear). Whether JUCE's WAV reader zero-fills on short reads is **unverified**.
  - No `isfinite` check. A 32-bit float WAV containing NaN or Inf becomes an IR with NaN, so the convolution output is NaN. The post-cab IIR (`PostEq`, when enabled) then latches NaN (CR-012).
  - `Normalise::no` at `CabIR.h:77,81`: a full-scale broadband IR of seconds length can give tens of dB of gain with no limiter after it.
  - There is no output limiter anywhere in the chain.
- **Failure scenario:** The user picks an unusual or damaged WAV. The host process grows by hundreds of MB, or the plugin emits NaN or a very loud signal.
- **Fix:**
  - Do all length math in `int64` and cap before casting: `len = (int) jmin<int64>(reader->lengthInSamples, maxLen)`.
  - Cap `dstLen` (or `maxLen * rate / srcRate`) and reject `srcRate` outside, say, 8 kHz to 384 kHz.
  - Check the `read` result.
  - Reject the buffer if any sample is non-finite.
  - Peak- or RMS-normalise the user IR (`Normalise::yes` or a manual gain).
  - Consider a soft output ceiling.

### CR-008 — No guard when the host block size exceeds the prepared maximum (heap overflow in CabIR and Oversampling)
- **Severity:** High (memory corruption triggered by host behaviour; host-dependent)
- **Where:**
  - `Source/DSP/CabIR.h:25-26` (scratch sized to `maximumBlockSize`) with `:172-173` (`getSubBlock(0, numSamples)`), then `copyBlock`
  - `Source/DSP/OversampledStage.h:34` (`initProcessing(maximumBlockSize)`) and `:67` (`processSamplesUp(baseBlock)`)
  - `PluginProcessor.cpp:303` (`processBlock` starts with no check against `prepared` block size)
- **What is wrong:** `processBlock` accepts any `numSamples`. `getSubBlock` and `Oversampling::processSamplesUp` only `jassert` in debug. In release, a block larger than the `samplesPerBlock` passed to `prepareToPlay` writes past `closeScratch`, `offScratch` and the oversampling buffers. Some hosts and offline renderers do exceed the declared maximum, although the VST3 specification forbids it.
- **Failure scenario:** A host prepares with 512 and sends 1024. Heap corruption in the host process.
- **Fix:** Store `maxBlock` at prepare. In `processBlock`, split the buffer into chunks of at most `maxBlock` samples (sub-buffer loop), or grow the buffers outside the audio thread and fall back to a safe bypass. Add a test processing 2x and 1.5x the prepared size.

---

## Medium

### CR-009 — Noise gate window is a 4096-sample box, not the documented 5 ms; `winLen` is dead; the reference test mirrors the bug
- **Severity:** Medium (confirms the known item; documented acceptance D-001m, but the code comments and plan still say 5 ms)
- **Where:** `Source/DSP/NoiseGate.h:45` (`winLen` computed and never used), `:3-4` and `:54` and `:102` (comments claim 5 ms), `:62-68` (`fifoSize = 4096` is the real window), `tests/MutationTests.cpp:129-171` (`GateReference`)
- **What is wrong:**
  - The detector window is 4096 samples. That is 85 ms at 48 kHz, 93 ms at 44.1 kHz and 21 ms at 192 kHz, so the behaviour depends on the sample rate.
  - `winLen` is a never-read local. Clang and gcc warn (`-Wunused-variable`), and JUCE's recommended flags may fail the build under warnings-as-errors (**unverified**).
  - `GateReference` in `MutationTests.cpp` re-states the same 4096 FIFO and the same `fifoSize`, and the comment says "5 ms-class". A reference test that is a copy of the implementation checks that the code did not change. It cannot detect that the spec (5 ms) is violated.
  - Off by one: after the FIFO fills, the sum holds 4095 samples (the oldest is subtracted at `:65` after `fifoPos` advances) but is divided by 4096 (`:67`). The effect is -0.002 dB and is harmless.
- **Failure scenario:** The note-onset gate opens about 5 times later than designed at 48 kHz. Behaviour differs by sample rate. The plan-level 5 ms claim is wrong in the shipped product.
- **Fix:** Either implement `winLen` (size the FIFO from `sampleRate * 0.005` up to a bound and use `n = winLen`), or change the spec and comments to the real behaviour and make the window a fixed time via a one-pole mean-square follower. Make the reference test independent (a direct-form RMS over the last N samples) so it fails if the spec window is violated.

### CR-010 — Gain smoothing is a per-block staircase and unequal per channel; the audit document overstates it
- **Severity:** Medium
- **Where:** `PluginProcessor.cpp:310-322` (input gain), `:404-415` (master), also `:329` (gate threshold) and `:459`-style `getNextValue(); skip(n-1)` pairs throughout; claim in `validation/automation-audit.md` (table row "`master` per-sample smoothed")
- **What is wrong:**
  - For input gain and master, `getNextValue()` is taken once and `FloatVectorOperations::multiply` applies that single value to the whole block for channel 0. Then `skip(numSamples-1)` and a second `getNextValue()` gives the end-of-block value for channel 1.
  - During a ramp, L and R therefore have different gains (a transient stereo imbalance) and the gain is a staircase, not a ramp.
  - D-006 and the audit say per-sample. At 1024-sample blocks the step is about a whole 20 ms ramp, so large automation changes are audible as steps.
  - T-010 passes because its bound is loose (3x steady-state delta).
- **Fix:** Use `applyGain` per sample from the smoothed value (`getNextValue()` inside the loop on both channels, with a shared gain per sample), or use `SmoothedValue::applyGain(buffer, n)` semantics for all channels from one smoother copy. Fix the audit wording.

### CR-011 — The post-EQ toggle crossfade is armed after the crossfade decision, so it runs a block late
- **Severity:** Medium
- **Where:** `PluginProcessor.cpp:303-305` (`doCrossfade` evaluated) versus `:392-396` (post-EQ toggle sets `crossfadeSamplesLeft`)
- **What is wrong:** When `post-eq-on` flips, the new filtered or unfiltered audio plays unfaded in that block. Only on the following block is the 5 ms crossfade applied, blending the new output with the previous block's output, which has no useful relation to the toggle. The documented "stepped params crossfade" does not hold for this parameter.
- **Fix:** Detect the toggle before computing `doCrossfade` (move the check above line 303), and keep the old-state output to blend against. Add a test: toggle post-EQ with a loud sine and check the largest sample-to-sample delta in that block.

### CR-012 — No NaN/Inf guard, no `AudioProcessor::reset()`, `getTailLengthSeconds()` is 0
- **Severity:** Medium
- **Where:** `PluginProcessor.cpp:136` (tail 0), absence of `reset()` override (`PluginProcessor.h`), `Preamp.h:95-157`, `NoiseGate.h` (`envelope`)
- **What is wrong:**
  - Any NaN or Inf from the host input or from the IR (CR-007) enters biquads and the sag `env` and persists, because IIR states and `env` are never cleared except by `prepareToPlay`. The plugin outputs NaN or silence until the host re-prepares.
  - Hosts call `reset()` on transport stop or start. It is not overridden, so convolution tails, filter memory and the gate state ring across stops.
  - `getTailLengthSeconds()` returns 0 although the cab convolution (a user IR up to 30 s) and the sag envelope have tails. Hosts that stop rendering after the clip ends will cut them.
- **Fix:**
  - Override `reset()` to call `gate`, `boost`, `preamp`, `toneStack`, `powerAmp`, `postEq`, `osStage.reset()` and `cabIR.reset()`.
  - Sanitise input at the top of `processBlock` (replace non-finite samples with 0), and sanitise the output.
  - Return a real tail length for the loaded IR.

### CR-013 — Preset save: path traversal, silent overwrite, ignored failure, wrong selection afterwards
- **Severity:** Medium
- **Where:** `Source/Presets/PresetManager.cpp:320-345` (`saveUserPreset`), `Source/PluginEditor.cpp:68-77` (save handler)
- **What is wrong:**
  - `name` goes straight into `dir.getChildFile(safeName + ".grindpreset")`. A name such as `../../Documents/x` or an absolute path resolves outside the preset directory. This is a write-anywhere (with the fixed extension and XML content) primitive controlled by the label text.
  - An existing preset with the same name is silently overwritten (data loss). Names with `:*?` fail on Windows and return -1, which the editor ignores.
  - After save, the editor selects `getNumPresets() - 1` (`PluginEditor.cpp:75`). `RangedDirectoryIterator` order is filesystem order, so the new preset is not necessarily last, and an overwrite adds no new entry. The combo box can show the wrong preset, and `currentPresetIndex` is not updated.
- **Fix:** Reduce the name to a file name with `File::createLegalFileName(name)` and strip separators. Check that `file.getParentDirectory() == dir`. Ask before overwriting (or append a suffix). Return the saved index from `saveUserPreset` and use it in the editor. Surface a failure to the user. Sort the user list.

### CR-014 — Preset and state parsing: no size or nesting limits; "drop unknown" comment contradicts the code; no root-type check
- **Severity:** Medium (most points **unverified** against JUCE internals)
- **Where:** `PresetManager.cpp:216` (`loadFileAsString`), `:244-283` (`tolerantParse`), `PluginProcessor.cpp:491-509`
- **What is wrong:**
  - `loadFileAsString()` reads an arbitrarily large file into memory.
  - `XmlDocument::parse` and `ValueTree::fromXml` are recursive. A user `.grindpreset` with tens of thousands of nested elements may overflow the stack of the host's thread (unverified). The fuzz generator (`tests/fuzz_preset.py`) has no deep-nesting mutation.
  - `tolerantParse` comment (`:252-253`) says unknown children are dropped. The `else` branch at `:276-279` copies them, so arbitrary subtrees from a preset or session state are stored in the state tree and re-saved into the host project.
  - The root type is not checked against `apvts.state.getType()` in either `loadPresetFromXml` or `setStateInformation`. A foreign root becomes the APVTS state root (JUCE behaviour here is unverified).
- **Fix:** Cap the file size (for example 1 MB). Reject documents deeper than a small limit with a pre-scan. Drop non-`PARAM` children as the comment says. Require `xml->hasTagName(apvts.state.getType())`. Add deep-nesting and wrong-root cases to the fuzz script.

### CR-015 — IR loader accepts every `registerBasicFormats()` decoder; the offline resample duplicates and degrades `dsp::Convolution`'s own
- **Severity:** Medium
- **Where:** `UserIRLoader.cpp:46-47` (`registerBasicFormats`), `:74-91` (resample), `CabIR.h:75-81` (passes `userIrRate` to `Convolution`)
- **What is wrong:**
  - The comments say WAV or AIFF only. `registerBasicFormats()` also enables FLAC, Ogg Vorbis, and on Windows or macOS the OS codecs (Windows Media, CoreAudio). Untrusted files therefore reach several third-party decoders for no product reason.
  - `ResamplingAudioSource` (Lagrange interpolation) does no anti-alias filtering when downsampling, for example a 96 kHz IR on a 48 kHz host. `Convolution::loadImpulseResponse(buffer, sampleRate, ...)` already resamples to the playback rate. The loader's resample is both redundant (the work happens twice) and lower quality. `raw` is also allocated and read and then discarded in the resample branch, doubling peak memory.
- **Fix:** Register only `WavAudioFormat` and `AiffAudioFormat`. Pass the native-rate buffer and its rate to `CabIR` and let `Convolution` resample. Delete the resample branch.

### CR-016 — The user IR slot is not reachable from the UI and is not persisted
- **Severity:** Medium (feature advertised in README and CHANGELOG)
- **Where:** `PluginProcessor.cpp:450` (`requestUserIrLoad`, called only by tests); `PluginEditor.cpp` (no file chooser); `PluginProcessor.cpp:485-509` (state holds no IR path)
- **What is wrong:**
  - Only tests call `requestUserIrLoad`.
  - Selecting "User IR" in the cab box falls back silently to the last factory cab (`CabIR.h:46-48`).
  - Even with a loader button, a saved project would restore "User IR" with no IR.
  - The header says "the callback fires on the message thread", but production passes `[](bool){}`, so the UI cannot report success or failure.
- **Fix:** Add a "Load IR..." button with `FileChooser` (async), store the path (or embed the IR) in the state, reload it in `setStateInformation`, and show load failures. Otherwise remove the user slot from the v1 list and README.

### CR-017 — CI workflow hardening: default token permissions, mutable action tags, `curl | sudo bash`, floating tool installs
- **Severity:** Medium
- **Where:** `ci.yml` (no `permissions:` block), `release.yml` (no `permissions:`), `ci.yml:219` and `:220` (Mull install), all `uses:` lines
- **What is wrong:**
  - Neither workflow sets `permissions:`. The `GITHUB_TOKEN` gets the repository default, which can be read-write. CI needs `contents: read`. Only `draft-release` needs `contents: write`.
  - Actions are referenced by mutable tag: `actions/checkout@v4`, `actions/upload-artifact@v4`, `actions/download-artifact@v4`, `softprops/action-gh-release@v2` (third party), `Tracktion/pluginval/validate@v2.0.0-beta3` (third party, a pre-release). None is pinned to a commit SHA. The release job holds the write token and runs the third-party `softprops` action.
  - `curl -1sLf 'https://dl.cloudsmith.io/.../setup.deb.sh' | sudo -E bash` runs an unpinned remote script as root with the runner environment. `apt-get install mull-19` has no version, so it floats (newest Mull is 0.36.0 while the pin is the "0.34.0 line"; see CR-020).
  - `windows-latest` and `macos-latest` float, which already affects the T-050 variance.
- **Failure scenario:** A compromised or re-tagged third-party action in the release job obtains a write token. A changed Cloudsmith script runs as root on the mutation job.
- **Fix:** Add top-level `permissions: contents: read`, with `contents: write` only on `draft-release`. Pin third-party actions by full commit SHA (with the tag in a comment) and use Dependabot for actions. Pin the Mull package version (`mull-19=0.34.x`) and verify the setup script by checksum, or install from a pinned .deb.

### CR-018 — "Freeze manifest check" hashes only `tests/TEST_SPEC.md`, not the frozen test code
- **Severity:** Medium (documented in D-001f, but the gate name over-promises)
- **Where:** `tests/FROZEN_MANIFEST.sha256` (one line, `TEST_SPEC.md`), `.github/workflows/ci.yml:42-44`
- **What is wrong:** `UnitTests.cpp`, `TestSupport.h`, `parameter_ids.txt` and `fuzz_preset.py` are not hashed. The suite's executable content can change without the "frozen manifest" check noticing. Two edits to `UnitTests.cpp` are already logged as D-001f. The CI step cannot distinguish a legitimate bring-up fix from a weakened assertion.
- **Fix:** Add the frozen test sources and fixtures to the manifest, with a recorded post-bring-up hash. Changes then require an explicit manifest update, which is reviewable. Keep `MutationTests.cpp` out, since it is explicitly non-frozen.

### CR-019 — Test coverage gaps (code paths that no frozen or mutation test exercises)
- **Severity:** Medium
- Concrete gaps (found by reading the tests against the code):
  1. **State restore after `prepareToPlay`** (CR-002): T-009 and the fuzz tail only compare parameter values on a never-prepared processor.
  2. **Distinct L/R content** and per-channel independence (CR-004): every test feeds the same sample to channel 0 and channel 1 (`UnitTests.cpp:181`, `:234`, `:283`, `:331`, `:509`, `:733`, `:808`).
  3. **Mono layout (1-channel buffer)** and `isBusesLayoutSupported`: no test builds a 1-channel buffer for `processBlock`.
  4. **Block larger than prepared size, or a prepared size smaller than 8192** for CabIR scratch (CR-008).
  5. **Runtime oversampling switch under the audio thread**: only the bench harness toggles it, not an assertion-bearing test (`BenchMain.cpp:167,200`). The `applySteppedParams` OS branch (`PluginProcessor.cpp:239-251`) is otherwise untested.
  6. **Gate with 4096 or more samples of exact digital silence after loud input** (CR-003). `burstThenQuiet` uses a 0.002 sine.
  7. **Gate in the full processor**: T-004 runs with the gate both on and off, but only on silence. No processor-level test checks that the gate attenuates.
  8. **Post-EQ toggle crossfade** (CR-011).
  9. **UserIRLoader:** concurrent or superseding requests, destructor with a load in flight, `take()` after a failed load, `maxLen` truncation (`:66`), zero-length file, mono or more than 2 channels, extreme sample rates, NaN content, callback invocation, `isBusy()`. Only same-rate, a 1 Hz delta, half-rate and a missing file are covered (`MutationTests.cpp:622-668`).
  10. **T-012 leg (c) assertion is vacuous:** `expect(proc.isUserIrApplied())` after a corrupt load (`UnitTests.cpp:794`) tests a flag that is set on the first successful load and never cleared. It cannot show the previous IR is still active. A test should compare output before and after the failed load.
  11. **PresetManager save path:** `saveUserPreset` with illegal or traversal names, overwrite, an unwritable directory, and `loadPreset` of a user file (T-008 covers a round trip only, `UnitTests.cpp:352+`).
  12. **`Convolution` hand-off timing:** no test asserts that `applyPending` leaves no unapplied pending flag after a user-IR swap at a changed sample rate (`CabIR::prepare` branch `:29-34`).
  13. **No ThreadSanitizer job.** The ASan and UBSan job (`ci.yml:149-193`) cannot find the loader and audio-thread races in CR-006 and CR-002.
- **Fix:** Add tests for items 1 to 3, 6, 10 and 11 first, since they would have exposed CR-002, CR-003, CR-004 and CR-013. Add a TSan job running `ir_loading` and `userir_loader`.

---

## Low

### CR-020 — Outdated or superseded dependencies (compared with `gh api`, 2026-10-31)
- **Severity:** Low
- JUCE: pinned `8.0.16-security`, commit `ea77444750de45e036e0279bb0291144bf8e3688`. The tag dereferences to exactly that commit and is the newest 8.x release (published 2026-10-13). The newest overall is `9.0.4` (2026-10-12). Staying on 8.x is the deliberate decision D-001 (9.x has breaking VST3 client changes). **OK.** The 9.0.4 notes also mention the malformed-String hardening already contained in 8.0.16-security.
- pluginval: `v2.0.0-beta3` is the newest tag (pre-release, 2026-10-14). The newest stable is `v1.0.4`. The pin matches D-001i. **OK, but a pre-release gate.**
- `actions/checkout@v4`: newest `v7.0.1` (v4 line ended at `v4.4.0`). The v4 action runs on `node20`, the v7 action on `node24`.
- `actions/upload-artifact@v4`: newest `v7.0.2`.
- `actions/download-artifact@v4`: newest `v8.0.2`.
- `softprops/action-gh-release@v2`: newest `v3.0.3` (v2 line ended at `v2.6.2`). Also `node20`.
- Mull: pinned "0.34.0 line", but the install is unpinned. The newest is `0.36.0` (0.34.2 is the last 0.34 patch), so CI may run a different version from the one recorded in `plan/ENVIRONMENT.md`.
- **Fix:** Move actions to the current majors after pinning by SHA (CR-017). Pin Mull to an exact package version.

### CR-021 — `NoiseGate::reset()` does not clear `fifo` and `fifoPos` (refuted as a functional bug)
- **Severity:** Low
- **Where:** `Source/DSP/NoiseGate.h:25-32`
- **Verified analysis:** `reset()` zeroes the sum and sets `rmsWindowFill = 0`, but leaves `fifo` and `fifoPos`. Stale entries are never subtracted: the first subtraction happens only once `rmsWindowFill == 4096`, and by then all 4096 slots have been overwritten. So the stale FIFO has no audible or numeric effect. It is an incoherent reset that will break if someone changes the fill logic. `smoothedThresholdDb = -60` in `reset()` is also meaningless, since the threshold is set every block.
- **Fix:** `std::fill(std::begin(fifo), std::end(fifo), 0.0f); fifoPos = 0;` in `reset()`.

### CR-022 — Dead or unused code
- **Severity:** Low
- `NoiseGate.h:45` `winLen` (CR-009), `:35` `maxBlockSize` parameter ignored (`ignoreUnused`).
- `PluginProcessor.h:111` and `PluginProcessor.cpp:170` `postEqScratch` is allocated and never used.
- `PluginProcessor.cpp:224,226` `pullSmoothParams(int numSamples)` takes and ignores `numSamples`.
- `OversampledStage.h:22,74` `channels` is stored and never read; `:50` `isHighQuality()` and `:53` `getInternalMaxBlockSize()` are unused.
- `PluginProcessor.h:69` `isCabIrUserPending()` has no caller. `PowerAmp.h:287` `getEnvelope()` is used by tests only.
- `PluginEditor.h:39` `presetBoxAttachment` ("unused slot kept for clarity").
- `UserIRLoader.cpp:99` the generation check (CR-006) is dead.
- `Voicings.h:49` `defaultCab` is used only by the bench harness (`BenchMain.cpp:168,256`). Choosing a voicing in the plugin does not select its cab, contrary to the field comment ("default cabinet").
- `PresetStatePort::setValueNotifyingHost` (`PresetManager.h:27-30`) exists only so a test mock can count calls, and production code must never call it. This is a test seam leaked into the production interface.
- **Fix:** Delete, or wire up `defaultCab` if it is intended.

### CR-023 — Duplicated BinaryData dispatch
- **Severity:** Low
- **Where:** `CabIR.h:206-232` (16 symbol macros and `juce::String` compares), `Parameters.h:274-288` (two parallel name tables), `PresetManager.cpp:108-154` (macro switch of 24 cases plus a name table)
- **What is wrong:** Three hand-maintained parallel tables map index to resource. `juce_add_binary_data` already generates `BinaryData::getNamedResource(name, size)` and `namedResourceList`. Adding a cab or preset needs edits in several places, and a missed one is only caught at runtime.
- **Fix:** Use `BinaryData::getNamedResource` with the generated name, and derive counts from `BinaryData::namedResourceListSize` or a single table.

### CR-024 — String-keyed parameter lookups on every block
- **Severity:** Low
- **Where:** `PluginProcessor.cpp:9-11,20-24` (`norm`, `choiceIndex`), called about 24 times per block
- **What is wrong:** Each call does a by-name lookup in the APVTS. It is allocation-free but wasteful at small buffer sizes.
- **Fix:** Cache `std::atomic<float>*` and `RangedAudioParameter*` once in the constructor.

### CR-025 — Cross-thread plain-variable reads
- **Severity:** Low
- **Where:** `PluginProcessor.h:63` `userIrApplied`, `CabIR.h:127-141` getters (`lastUserValid`, `currentFactory`, `hasPendingUser`), read by tests and UI threads while the audio thread writes them
- **What is wrong:** These are non-atomic and are formally data races. They are benign in practice on current compilers and only expose status.
- **Fix:** Make them `std::atomic<bool>` or `std::atomic<int>` with relaxed ordering.

### CR-026 — Stepped-change crossfade blends against the previous block's samples at the same index
- **Severity:** Low (design limitation; D-001g already fixed the uninitialised read)
- **Where:** `PluginProcessor.cpp:419-441`
- **What is wrong:** `cur[i] = t*cur[i] + (1-t)*prev[i]` mixes sample i of the current block with sample i of the previous block. That is audio from a different time (the previous block), not the old-settings render of the same samples. The result is a short comb or garble rather than a clean crossfade. It is bounded by 5 ms, so low impact.
- **Fix:** Render the old chain in parallel for the crossfade window, or fade out and in with a short duck.

### CR-027 — CI and release workflow efficiency and gating
- **Severity:** Low
- **Where:** `ci.yml:46-47` and `:111-112` (the full `ctest` already runs the `bench` test registered in `tests/CMakeLists.txt`, then the bench runs a second time); `release.yml:27-28`
- **What is wrong:**
  - The benchmark (timeout 900 s) runs twice per OS per push.
  - `release.yml` builds on any `v*` tag without requiring a green CI run on that commit and without the JUCE-pin and frozen-manifest steps.
  - The release job runs `ctest`, including the 900 s bench, with no gate on its result.
  - macOS artifacts are not notarised (documented as unsigned-first in ENVIRONMENT.md).
- **Fix:** Exclude `bench` from the first ctest (`-E bench`). In `release.yml`, require the CI result for the tagged commit or repeat the pin, manifest and pluginval steps.

### CR-028 — Master at 0 is not silent
- **Severity:** Low
- **Where:** `PluginProcessor.cpp:211` (`jmax(0.0001f, master)`)
- **What is wrong:** The floor exists because multiplicative smoothing needs a positive target. Master at zero therefore leaves a -80 dB signal (times the voicing trim) instead of mute. Users expect a mute at 0.
- **Fix:** Fade to true zero (use `Linear` smoothing for master, or special-case 0 with a short ramp).

---

## Checked and found fine

- No secrets, tokens or credentials in code or configuration (grep for common patterns). The repo relies only on the default `GITHUB_TOKEN`.
- JUCE pin: submodule commit `ea77444…` equals the `8.0.16-security` tag target, and it is the newest 8.x release. `JUCE_USE_CURL=0`, `JUCE_WEB_BROWSER=0` and `JUCE_VST3_CAN_REPLACE_VST2=0` (before `add_subdirectory(JUCE)`) are set as planned.
- Choice-parameter decoding (`choiceIndex`, lround of `convertFrom0to1(getValue())`) is correct, and the earlier (int) truncation defect is fixed.
- `getRawParameterValue` atomics are used for all parameter reads on the audio thread, and nothing parameter-related holds a lock.
- `PresetManager::loadPreset` out-of-range indices return false; the 24-preset count is `static_assert`ed; replaceState (not per-parameter notification) is used for loads (D-007).
- `OversampledStage` Eco mode is a true pass-through with zero latency; latency is read from `Oversampling::getLatencyInSamples()`; the bench harness checks reported against measured latency.
- `UserIRLoader` failure path: a failed load clears `ready`, so the previous IR stays in `CabIR`.
- The `CabIR::applyPending` user-IR branch clears `pendingFactory`, which closes the stale fallback case documented in its comment.
- No `new`/`delete` leaks, no exceptions thrown in `processBlock`, `ScopedNoDenormals` is present.
- CI Rule 14 post-conditions that do work: artifact `if-no-files-found: error`, the Mull score read from `mull.sqlite` with a minimum mutant count, `set -o pipefail` with `tee` in sanitiser runs, `ALL TESTS PASSED` count assertion.
- `pull_request` (not `pull_request_target`) is used. No untrusted `github.event` fields are interpolated into `run:` blocks. `GITHUB_REF_NAME` is used only as a shell variable.
- `tools/ci/check_bench.py` handles missing, empty and unparsable input, missing voicings and unknown runners, and has self-tests for both pass and fail cases.
