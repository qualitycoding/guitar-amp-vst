# ADVERSARIAL REVIEW (S-ADVERSARIAL, planning-protocol v3.6 section 3.8)

Reviewer: fresh-context adversarial reviewer. Scope: branch `impl-run-20261027T120000Z-guitar-amp-vst`, read-only.
Method: code reading, JUCE 8.0.12 source read through `gh api` (the pinned 8.0.16-security tree is not checked out,
so JUCE behaviour is quoted from 8.0.12), and small python3 emulations (scripts were kept in /tmp, not in the repo).
Nothing was compiled or run. Where a claim rests on emulation or on JUCE source rather than on an executed test,
the entry says so.

Severity scale used (the text of Global Rule 4 was not in the files supplied; this mapping is the reviewer's):
Critical = data loss, host crash on ordinary use, or loud/undefined output on ordinary use;
High = host crash or silent wrong behaviour reachable from a plausible hostile or corrupt file, or an ordinary-use failure of a core requirement;
Medium = audible defect or broken feature on a reachable path, or a frozen check that cannot fail;
Low = cosmetic, operational, or robustness gap with a workaround.

## Already reported by another reviewer (deliberately NOT re-reported here)

Gate NaN latch after loud-then-zeros; `setStateInformation` leaving DSP on stale settings; L/R sharing one set of filter state;
real-time allocations in `processBlock`; `UserIRLoader` holding its lock for the whole load; unvalidated user-IR files and the
uncapped resampled length; no guard for blocks larger than prepared; CI gates using `! grep` under bash -e.
Overlaps are flagged inside individual entries (AR-003).

## Summary table

| ID | Severity | Area | One line | Test or accepted risk |
|----|----------|------|----------|-----------------------|
| AR-001 | High | Preset / state parsing | `value="nan"` in a preset or session blob becomes a NaN parameter; chain goes NaN, and on Windows a NaN choice index reads `voicings[]` out of bounds in `prepareToPlay` | New test (extend T-013 corpus) |
| AR-002 | Medium | Stepped-change crossfade | The 5 ms crossfade starts from the previous block's first sample, not its last, so every voicing/cab/oversampling/PostEQ switch adds a discontinuity of about 6-7x the steady 1 kHz sample delta | New test |
| AR-003 | Medium | Mono layout, High mode | In a 1-channel bus JUCE's oversampler still hands the callback 2 channels; the phantom zero channel is run through the same filters after channel 0 each block | New test (mono layout) |
| AR-004 | Medium | Smoothing (D-006) | Input gain, master, drive, tone and mic/mix smoothers are sampled once per block and applied as a constant: staircase gain, and R is one block ahead of L; T-010 renders with 1-sample blocks so it cannot see it | New test (spec-literal T-010) |
| AR-005 | Medium | Sample-rate range | Float32 biquad coefficients are unstable below about 11 kHz (Eco) and at 768 kHz (High); marginal at 384 kHz; nothing rejects the rate | Accepted risk, or add a rate guard |
| AR-006 | Medium | User IR slot | No UI loads a user IR, the IR is not persisted in state, and "User IR" in the cab list silently plays the previous factory cab | Accepted risk or scope fix |
| AR-007 | Medium | D-007 / T-011(b) | Preset load still notifies the host per parameter (JUCE `replaceState`); T-011(b) mocks `replaceState` to a no-op so it cannot detect this; the cited `TEST_CHALLENGE.md` does not exist | Process fix + T-032 stays gating |
| AR-008 | Low | Preset save | Name is used as a path (traversal, absolute paths), existing presets are overwritten silently, failure is ignored and the wrong list item is selected | New test |
| AR-009 | Low | Preset directory | On macOS `userApplicationDataDirectory` is `~/Library`, so presets go to `~/Library/GrimAmp`, not the `~/Library/Application Support/GrimAmp` that D-008 and README state | Fix or amend docs |
| AR-010 | Low | Host reset / tail | No `AudioProcessor::reset()` override and `getTailLengthSeconds()==0`: host reset leaves old filter, gate, convolution and `prevBlock` content | Accepted risk |

No Critical break was confirmed. One High (AR-001).

---

## Confirmed breaks

### AR-001 (High) NaN parameter values from preset/state XML

- Where: `Source/Presets/PresetManager.cpp:267-273` (value copied verbatim), `Source/PluginProcessor.cpp:20-24` (`choiceIndex`), `:162` (`gramp::voicings[currentVoicing]` unguarded), `:500-504` (mirrors assigned unclamped), `:253-255` (compare uses raw value, then clamps), `Source/DSP/Preamp.h:46` (`jlimit` passes NaN). `tests/fuzz_preset.py:30` inserts only `9*40`, `1e309`, `-1e309`, `2147483648` and no `nan`.
- Trigger: a `.grindpreset` file, or a session blob, containing `<PARAM id="drive" value="nan"/>` (or `id="amp-voicing" value="nan"`). JUCE parses "nan" to NaN (`CharacterFunctions::readDoubleValue` has explicit NaN handling, per its own unit test). APVTS `setDenormalisedValue` -> `normalise()` -> `NormalisableRange::convertTo0to1` -> `clampTo0To1` is `jlimit`, which returns NaN for NaN in release builds (only a `jassert` fires in debug).
- Observed:
  - Float param: NaN reaches `Preamp::setDrive` and the smoothers, then every IIR state, which latches NaN until `prepareToPlay`. Output is NaN. (`AudioParameterFloat.cpp` could not be fetched, so the step from `setValue` to the stored value is inferred from `NormalisableRange`, which was read.)
  - Choice param: `std::lround(NaN)`. On LP64 (Linux/macOS) the `(int)` of `LONG_MIN` is 0, which is benign. On Windows `long` is 32-bit and the result is expected to be `INT_MIN`. Then `setStateInformation` stores it unclamped in `currentVoicing`, and the next `prepareToPlay` evaluates `voicings[INT_MIN]`: an out-of-bounds read of about 2.1e9 x 104 bytes below the table, which is a host crash. This Windows result is from reasoning about the C runtime; it was not executed.
  - If the NaN arrives via `replaceState` instead (preset), `pendingVoicing` stays out of range, `pendingVoicing != currentVoicing` is true every block, and `setVoicing` (which resets filter states) plus a new crossfade run on every block: continuous crackle.
- Expected: a malformed value is rejected or clamped; T-013 says the processor "remains in a valid state".
- Action: new test (add `nan`, `inf`, `-nan` and `value=""` mutants to the T-013 generator, then check finiteness and `voicings[]` indices after load and `prepareToPlay`); clamp in `choiceIndex` and `tolerantParse`.

### AR-002 (Medium) Stepped-change crossfade is time-misaligned

- Where: `Source/PluginProcessor.cpp:419-441` (crossfade), `:444-447` (prevBlock capture), `:303-305` (`doCrossfade`); filter resets in `Source/DSP/Preamp.h:32-33` and `Source/DSP/PowerAmp.h:218-233` (`setVoicing` calls `prepare`, which resets states).
- Trigger: change amp voicing, cab, oversampling or the Post EQ toggle during playback of a steady tone, e.g. 1 kHz sine, 48 kHz, 512-sample blocks.
- Observed: at `i = 0` the output is `prev0[0]`, the first sample of the previous block, while the previous block's last output was `prev[N-1]`. Blending then continues sample-index against sample-index, which mixes audio from two different times. Emulated for a 1 kHz sine at amplitude 0.5: the jump at the start of the crossfade is 0.40, 6.1x the steady maximum sample-to-sample delta of 0.065 (7.1x for block sizes 64 and 256, 1.0x for 480). A voicing change additionally zeroes the preamp and power-amp filter states mid-signal. If block sizes vary, `prevBlock` is resized each block (`:444`) and `doCrossfade` is silently skipped (`:304`).
- Expected: D-006 and S-012 say stepped parameters switch "without clicks" (the T-010 bound is 3x).
- Action: new test: render a steady sine, switch each stepped parameter, assert max delta <= 3x steady. T-010 covers only master, drive and treble.

### AR-003 (Medium) Mono bus layout in High mode runs a phantom second channel

- Overlap note: the root cause is related to the already-reported shared-state-across-channels issue, but the trigger and the effect differ (mono bus, no real second channel, channel 0 itself is damaged), so it is listed once here.
- Where: `Source/PluginProcessor.cpp:358-376` (loop over `osBlock.getNumChannels()`), `Source/DSP/OversampledStage.h:67-69`; JUCE `juce_Oversampling.cpp:62-65` (`getProcessedSamples` returns all `numChannels` = 2 channels) and `:92-107`/`:322-` (`processSamplesUp` writes only the input's channels).
- Trigger: `isBusesLayoutSupported` accepts mono/mono. In a mono-bus instance with oversampling = High, the oversampled block has 2 channels; channel 1 is all zeros.
- Observed: after channel 0's `4N` samples the same preamp, power-amp and tone-stack state objects process `4N` zeros, so channel 0 starts every block from decayed state. Emulation of the two preamp stages (Plexi, 192 kHz internal, 1024-sample blocks): error against a continuous reference up to 0.24, -7.8 dB re peak, repeating at block rate. The emulation excludes the tone stack.
- Expected: mono in -> mono out without block-rate artefacts. Eco mode is not affected (the block has 1 channel).
- Action: new test: mono-layout instance, High, compare against a Eco/High single-block render.

### AR-004 (Medium) Parameter smoothing is block-rate, not per-sample (D-006); T-010 cannot see it

- Where: `Source/PluginProcessor.cpp:308-323` (input gain: one `getNextValue`, constant multiply), `:405-416` (master), `:347-356` (drive/bass/mid/treble/presence/depth), `:380-383` (mic, mix), `:399-402`.
- Trigger: any large parameter change inside one block, e.g. a preset load changing master from 0 to 1 at block size 512, 48 kHz (20 ms ramp = 960 samples).
- Observed: master is applied as the constant 0.000101 for block 1, 0.0137 for block 2 (sample 513 of the multiplicative ramp), then 1.0: steps of +43 dB and +37 dB at block boundaries. For stereo, channel 1 uses the value taken after `skip(numSamples-1)`, i.e. one block later than channel 0, so L and R gains differ for the whole ramp. Smoothing is effectively a staircase of block size; at 2048-sample blocks it is no smoothing at all.
- Test gap: `tests/UnitTests.cpp:470-486` (T-010) steps the parameter once per 1-sample `processBlock`, so every block sees a fresh per-sample value. TEST_SPEC T-010 says "over one 512-sample block". This is a deviation from the frozen text not logged in DEVIATIONS.md (D-001f lists only other test changes).
- Expected: sample-accurate ramp (D-006) and the spec-literal T-010 bound.
- Action: new/strengthened test (single 512-sample block ramp); log the T-010 deviation.

### AR-005 (Medium) Float32 biquad design is unstable or marginal outside about 22-192 kHz

- Where: `Source/DSP/Preamp.h:26-29`, `Source/DSP/ToneStack.h:167-178`, `Source/DSP/PowerAmp.h:228-229,284-285`, `Source/DSP/Boost.h:214-217`, `Source/DSP/PostEq.h:291-294`; no rate check in `Source/PluginProcessor.cpp:151-189`.
- Method: a python emulation of JUCE's `IIRFilter.cpp` coefficient formulas (8.0.12) with float32 rounding at every operation, covering every voicing and knob extreme, Post EQ, boost. Stability margin = min(1-|a2|, 1+a2-|a1|).
- Result:
  - 8 kHz Eco: 20 unstable filter configurations (presence shelf 4200-5600 Hz is above Nyquist, as are others).
  - 11.025 kHz Eco: Frost presence shelf at 5600 Hz is above Nyquist (5512.5 Hz), unstable.
  - 8 kHz High: Boost low-pass at 4500 Hz and Post EQ high shelf at 4500 Hz exceed Nyquist (host-rate filters), unstable.
  - 768 kHz High (3.07 MHz internal): 18 configurations at margin <= 0 (low shelves at 80-120 Hz quantise to a1 = -2). The margin is within one float ulp, so the sign depends on libm rounding: treat as "unstable or on the edge".
  - 384 kHz High: minimum margin 6e-8 (marginal). 192 kHz High: 3e-7 (stable, thin; T-005 covers 192 kHz High).
  - 22.05 kHz-192 kHz: stable.
- Observed consequence: exponential growth in the unstable filters, which then latches NaN/Inf (see also AR-001's latching behaviour). A-010 states 44.1-192 kHz, which is why this is Medium rather than High, but nothing refuses or clamps other rates; hosts such as REAPER offer 8, 11.025 and 768 kHz.
- Action: accepted risk with a documented rate range, or a guard in `prepareToPlay` (clamp design frequencies below 0.45 x rate; design low shelves in double). Add 8 kHz, 11.025 kHz and 384/768 kHz to T-005's rate list if supported.

### AR-006 (Medium) User IR slot is a dead feature and silently misbehaves

- Where: `Source/PluginEditor.cpp` (no file chooser; `requestUserIrLoad` is called only from `tests/UnitTests.cpp:750-791`), `Source/PluginProcessor.cpp:484-507` (state holds no IR path), `Source/DSP/CabIR.h:48-62,119-126,32-38`, `Source/Parameters.h:172-181`.
- Observed:
  1. "User IR" is selectable in the cab list but nothing can load an IR. Selecting it with no IR keeps the previous factory cab (`CabIR.h:52-55`) while the UI shows "User IR". D-004 and the plan (PLAN.md:174-175) promise WAV/AIFF loading.
  2. The IR is not part of `getStateInformation`; a saved session with cab = "User IR" reloads as factory cab 1 (`prepare` sets `pendingFactory = 0`, `CabIR.h:35-36`).
  3. When an IR does arrive (test path), `setUserImpulseResponse` forces `hasPendingUser = true` regardless of the cab parameter, so the audible cab no longer matches the parameter; the next `prepareToPlay` (`selectCab` with a factory index, `CabIR.h:59-60`) reverts it silently.
- Consequence: the reachable product never exercises the user-IR code, so T-012 passes on a path no user can reach. Latent hazards of that code are in the other reviewer's list.
- Action: accepted risk or scope decision; if kept, record that the slot is non-functional in v1 and hide it from the choice list, or add UI plus persistence.

### AR-007 (Medium) D-007 is not met in a real host and T-011(b) cannot detect it

- Where: `Source/Presets/PresetManager.h:33-41`, `tests/UnitTests.cpp:572-618`; JUCE `juce_AudioProcessorValueTreeState.cpp:396-415,101-105,171-177` (`replaceState` -> `setNewState` -> `setDenormalisedValue` -> `setNormalisedValue` -> `parameter.setValueNotifyingHost`).
- Observed: JUCE's `replaceState` itself calls `setValueNotifyingHost` for each changed parameter, with no gesture. The knowledge entry K-20261027T120100Z identifies this pattern as what makes Live create automation lanes. In T-011(b), `MockPort::replaceState` is an empty function (`:581`), so the real `replaceState` never runs and `valueChanges` is always 0; the assertions `notifyCalls == 0` and `gestures == 0` hold trivially. The comment (`:607-613`) says the frozen wording "is challenged in TEST_CHALLENGE.md"; no such file exists in the tree or in git history (`git ls-files | grep -i challenge` is empty).
- Consequence: R-003 (preset loads spray automation lanes) is verified only by T-032, a manual Live test. Two Rule-14 problems: a check that cannot fail, and a reference to an artefact that does not exist.
- Action: create the missing `TEST_CHALLENGE.md` (or correct the comment); make T-011(b) use the real `ApvtsPresetPort` and assert on counted notifications; keep T-032 as a gate; record R-003 residual as open.

### AR-008 (Low) Preset save: name used as a path, silent overwrite, ignored failure

- Where: `Source/Presets/PresetManager.cpp:318-345`, `Source/PluginEditor.cpp:63-73`.
- Trigger: save with name `../../x` (resolves outside the presets directory because `File::getChildFile` resolves `..`), or an absolute path such as `/tmp/x` or `C:\x` (`getChildFile` returns the absolute path). The suffix `.grindpreset` is always appended, so arbitrary-file overwrite is limited to files with that suffix. The name is typed by the local user, so this is a self-inflicted path, not a remote one.
- Observed: the file is written outside the list directory, `rebuildList` does not list it, so `saveUserPreset` returns -1 although the write succeeded. The editor ignores the return value (`:68`) and selects the last list item (`:71`), which is not the saved preset (directory iteration order); a failed save (read-only directory) looks identical. A same-named preset is overwritten without confirmation, and may duplicate a factory name.
- Action: new test (names with separators, `..`, absolute path, reserved Windows names, over-long names); sanitise to a leaf file name.

### AR-009 (Low) macOS user-preset directory differs from the documented one

- Where: `Source/Presets/PresetManager.cpp:169-174`. JUCE `File::userApplicationDataDirectory` documents "On the Mac, it might be `~/Library`" (`juce_File.h:907-909`), so the code writes `~/Library/GrimAmp`; D-008 and `README.md:29-30` say `~/Library/Application Support/GrimAmp`. The mac source file was not fetched, so this rests on the header text.
- Consequence: documentation, installer and uninstall expectations are wrong on macOS; T-060 would catch it only if it checks the path.
- Action: use `~/Library/Application Support` explicitly on macOS (or amend the docs).

### AR-010 (Low) No `reset()` and zero tail

- Where: `Source/PluginProcessor.h` (no `reset()` override), `Source/PluginProcessor.cpp:136` (`getTailLengthSeconds` returns 0).
- Observed: when a host calls `AudioProcessor::reset()` (AU `AudioUnitReset`, some hosts on transport start), filters, gate window, power-amp envelope, convolution overlap and `prevBlock` keep their content; the next block can replay stale audio and any latched NaN (see AR-001) survives a reset. The convolution and reverb-like IR tail is truncated by the host once input stops because the reported tail is 0.
- Action: accepted risk or override `reset()` to call each stage's `reset()` and clear `prevBlock`.

---

## Unconfirmed hypotheses

- H-1 Deeply nested XML in a preset or session blob overflows the stack. `XmlDocument::readNextElement` recurses (`juce_XmlDocument.cpp:562`), `ValueTree::fromXml` recurses (`juce_ValueTree.cpp:1014-1015`), `tolerantParse` calls `createCopy()`; I found no depth limit in the code read. Input would be about 100k nested `<a>` elements (about 300 KB). Not executed; stack size and frame size unknown. High confidence, would be Medium-High.
- H-2 `UserIRLoader::request` calls `startThread()` and ignores its return value, which is false while the previous thread's handle is still open (`juce_Thread.cpp:157-173`); a request issued just after a run finishes can be dropped, leaving `loading == true`. `~Impl` uses `stopThread(2000)`, which force-kills the thread after 2 s (`juce_Thread.cpp:245-277`). Both touch the loader-lock area the other reviewer covered, so I treat them as extensions, not new breaks. Only reachable from tests today (AR-006).
- H-3 `setLatencySamples` is called from the audio thread (`PluginProcessor.cpp:199`, via `applySteppedParams:249`) when oversampling switches. Whether the VST3/AU wrapper defers the host notification safely was not read.
- H-4 Block size 1 with the zero-latency `dsp::Convolution`: the uniform partition size follows the maximum block size, so a 2048-sample IR could mean thousands of segments per sample. Not measured; T-034 covers only 64/256/1024.
- H-5 `prepareToPlay(0, n)` or `(sr, 0)`: no validation in `PluginProcessor.cpp:151-189`; `Boost`/`PostEq` design with rate 0 yields NaN coefficients; with a block size of 0, `closeScratch` is sized 0 and `getSubBlock(0, numSamples)` would overflow. Hosts do not normally do this.
- H-6 `AudioParameterFloat::setValue(NaN)` storing NaN: not read (404 on the fetched path); inferred from `NormalisableRange`. AR-001's Windows out-of-bounds read depends on MSVC `lround(NaN)` returning `INT_MIN`.
- H-7 Junk preserved by `tolerantParse` (`:276-279` copies all non-PARAM children, unknown PARAM ids stay in the tree) is written back by `getStateInformation` into every project, inflating session files from a hostile preset. Not quantified.
- H-8 Loaded presets change the APVTS state root type from "PARAMETERS" to the file's root tag ("Parameters" in the 24 factory files); `setStateInformation` has no type check, so recall works, but any code that finds children by root type would not. No such code was found.

## Attacked and held

- Bus layouts: anything other than mono/mono or stereo/stereo is rejected (`isBusesLayoutSupported`); extra buffer channels beyond 2 are ignored by `jmin`.
- `processBlock` with 0 samples, 0 channels, or before `prepareToPlay`/after `releaseResources` returns without touching state; repeated `prepareToPlay` re-sizes every buffer and clears `prevBlock`.
- Block size 1 and sizes below 240: crossfade arithmetic (`n = min(numSamples, crossfadeSamplesLeft)`, `inc` guarded by `jmax(1, ...)`) does not divide by zero or index out of range; `skip(0)` on the smoothers is safe.
- Digital silence from start (T-004): the gate sum starts at exactly 0; `ScopedNoDenormals` is present; the power-amp even-harmonic term is input-driven so there is no DC leak.
- Smoother recovery from a NaN target: JUCE `SmoothedValue` assigns `current = target` at the end of the ramp, so a transient NaN parameter clears after 20 ms (the latching is in the IIR states, not the smoothers).
- Factory presets: all 24 files carry exactly the 21 ids listed in `tests/parameter_ids.txt` with in-range values; choice values are stored denormalised as APVTS expects.
- Missing or unknown PARAM ids: unknown ids are ignored by `replaceState`, missing ones reset to defaults; non-XML or empty input returns false without throwing.
- `choiceIndex` converts through the parameter's own range (the earlier `(int)` bug is fixed) for all 7/9/2-entry choices.
- Empty or zero-channel IR handed to `dsp::Convolution`: JUCE `fixNumChannels` substitutes a unit impulse (`juce_Convolution.cpp:555-567`), so no crash.
- Destroying the processor while the loader thread runs: the callback captures nothing from the processor, and the Convolution background queue captures a `weak_ptr` (`:1000-1010`), so there is no dangling reference (the 2 s kill is H-2).
- Rapid cab switching: JUCE's `callLater` keeps only the newest pending command per engine, so a flood of switches does not overflow the queue (`:1004-1014`).
- BinaryData lookup: an unknown IR name leaves `data == nullptr`, which is guarded at `CabIR.h:102`; preset lookup returns false for a missing symbol.
- `saveUserPreset` with an empty or whitespace name falls back to "Untitled"; an unwritable directory makes `XmlElement::writeTo` return false and the function returns -1.
