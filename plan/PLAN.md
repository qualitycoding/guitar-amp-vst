# PLAN — GrimAmp: Rock & Metal Guitar Amp VST

**Status: READY** (pending A-001 repo push by the human)
**Profiles:** `software` (`software.deploys = true`)
**Counts:** claims 8 (verified 5, corroborated 2, single-source 1) · tests 20 (unit/dsp 13, host 2, live 5, security 2, performance 2, operational 1) · steps 20 · gates 3 (G-002, G-003, G-010) · risks 14 (Critical 1 and High 4 → all mitigated or resolved, Medium 7, Low 2) · prior knowledge: store `qualitycoding/knowledge` read at `09f34582…`; 78 items tag-matched, 9 applied (D-001 re-pinned to JUCE 8.0.15; `JUCE_VST3_CAN_REPLACE_VST2=0` added; GUI/CI/packaging hardening), 0 lessons in store · amendments: A-008 (store corrected to `qualitycoding/knowledge`), D-001 (JUCE 8.0.12 → 8.0.15, 9.x forbidden), DR-9 added, S-000 action 4 added, S-002 action 3 added

Every step follows the Rule 3.1 template. Independent steps may run concurrently unless they share an exclusive resource. The plan begins with S-000 and ends with S-RETRO then S-KNOW.

---

### S-000 Environment & Access Verification
- Tier: Haiku
- Profile: software
- Depends on: none
- Inputs: `plan/ENVIRONMENT.md`, `plan/ASSUMPTIONS.md`
- Actions:
  1. `git checkout -b impl-run-20261027T120000Z-guitar-amp-vst` from the generation branch; all implementation commits go there.
  2. Run every setup command in `plan/ENVIRONMENT.md` §1–2.
  3. Re-run the 0.1 checks from this environment: `gh auth status`, `git ls-remote https://github.com/qualitycoding/guitar-amp-vst.git`, push a no-op commit to the impl branch, verify Actions enabled, verify reachability of the hosts table.
  4. **Knowledge-store re-verification (DR-9, from the A-008 amendment):** `gh repo view qualitycoding/knowledge`; if readable, run the R0 matching rule (Appendix C) against this task's tags, record loaded entries in `research/PRIOR_KNOWLEDGE.md` (logged in `DEVIATIONS.md`), and apply prevention rules before S-001. If a loaded lesson invalidates a frozen test or locked decision → halt with `TEST_CHALLENGE.md`/`BLOCKED.md`.
  5. Create `EXECUTION_LOG.md` (one line per step attempt: UTC time, step ID, attempt, outcome, commit hash, note). Append to it in every later step.
- Outputs: `EXECUTION_LOG.md`, impl branch pushed.
- Evidence produced: none
- Done when: every check passes; push confirmed.
- Checkpoint: `state.json` ← phase=implementation, active_step_id=S-000, completed+=[S-000].
- On failure: halt; write `BLOCKED.md` listing exactly what is missing (credential, permission, host). No delivery work before this passes.
- Gate: none
- Relevant decisions/claims: A-001, A-008, DR-9, C-001
- Lessons applied: L-20261027T121000Z-wrong-knowledge-store (prevention rule implemented by action 4)
- Exclusive resources: git remote

### S-001 Repository & CI Scaffold
- Tier: Sonnet
- Profile: software
- Depends on: S-000
- Inputs: `plan/ENVIRONMENT.md`
- Actions:
  1. Create the repo skeleton: `CMakeLists.txt`, `Source/`, `tests/`, `.github/workflows/ci.yml`, `LICENSE` (GPL-3.0 per A-009), `README.md`, `.gitignore` (build/, .checkpoints/snapshots excluded from releases but `.checkpoints/` committed).
  2. Add JUCE as a git submodule pinned to tag 8.0.15: `git submodule add https://github.com/juce-framework/JUCE.git && git -C JUCE checkout 8.0.15`, then verify `git -C JUCE rev-parse HEAD` == `91ad83ae34a81e0833b1a2b0866f54846370ae53`. Evaluate the `8.0.16-security` patch tag (security lens, D-001); if adopted, update the hash pin and record in `DEVIATIONS.md`.
  3. CI workflow: matrix {windows-latest, macos-latest} × {Release}; steps: checkout w/ submodules → configure → build → ctest → freeze-manifest check (`sha256sum -c tests/FROZEN_MANIFEST.sha256`).
- Outputs: scaffolded repo; green CI on an empty plugin target.
- Evidence produced: none
- Done when: CI job builds and the manifest check passes.
- Checkpoint: completed += S-001.
- On failure: DR-1 (dependency/install), DR-7 (reachability). Rollback: `git reset --hard` to S-000 commit.
- Gate: none
- Relevant decisions/claims: D-001, C-001, C-005
- Lessons applied: none
- Exclusive resources: none

### S-002 JUCE Plugin Skeleton ("GrimAmp" passes audio & pluginval)
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: JUCE submodule
- Actions:
  1. `juce_add_plugin(GrimAmp ...)` in CMake: formats `VST3 AU Standalone`; `IS_SYNTH FALSE`, `NEEDS_MIDI_INPUT FALSE`; `VST3_CATEGORIES "Distortion"`; unique plugin codes; company name `qualitycoding`.
  2. Minimal `PluginProcessor` (pass-through gain) + `PluginEditor` with one slider; module list: `juce_audio_processors juce_dsp juce_gui_extra juce_audio_utils`.
  3. Add `add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)` at directory scope **before** `add_subdirectory(JUCE)` (K-20261009T092101Z: default =1 can hard-`#error` a plugin that never shipped VST2; K-20261009T092102Z: the Standalone target compiles the VST3 client too, hence directory scope).
  4. Add `juce_add_binary_data` stub (empty for now).
  5. Run pluginval strictness 5 locally on both OSes.
- Outputs: `Source/PluginProcessor.{h,cpp}`, `Source/PluginEditor.{h,cpp}`, build artefacts.
- Evidence produced: T-001, T-020 (initial pass)
- Done when: T-001 and T-020 pass locally and in CI.
- Checkpoint: completed += S-002.
- On failure: DR-2. Never lower strictness.
- Gate: none
- Relevant decisions/claims: D-001, C-003
- Lessons applied: none
- Exclusive resources: none

### S-003 Parameter Layout (APVTS)
- Tier: Opus
- Profile: software
- Depends on: S-002
- Inputs: D-005, A-007
- Actions:
  1. Write `Source/Parameters.h` defining every parameter once: input gain [-24..+24 dB], gate threshold [-80..0 dB] + gate on/off, boost on/off + boost amount [0..1], drive [0..1], bass/mid/treble [0..1], presence [0..1], depth [0..1], sag [0..1], amp voicing (choice: Tweed/Plexi/JCM800/Recto/5150/Chainsaw/Frost), cab IR (choice: 8 factory + user slot), mic position [0..1], IR mix [0..1], post-EQ on/off + low/high shelf gains, master [0..1], oversampling (choice: Eco/High).
  2. Freeze the exact IDs into `tests/parameter_ids.txt` (T-002's reference).
  3. Wire `AudioProcessorValueTreeState` with a `ParameterLayout` built from that header; implement `getStateInformation`/`setStateInformation` via `copyState`/`replaceState`.
  4. Register the UndoManager as nullptr (no in-plugin undo in v1).
- Outputs: `Source/Parameters.h`, processor state handling, `tests/parameter_ids.txt`.
- Evidence produced: T-002, T-003, T-009
- Done when: T-002, T-003, T-009 pass.
- Checkpoint: completed += S-003; variable_store["param_count"].
- On failure: fix and re-run; ID renames are forbidden after this step (D-005) — a needed rename halts as `BLOCKED.md`.
- Gate: none
- Relevant decisions/claims: D-005, A-007
- Lessons applied: none
- Exclusive resources: `Source/Parameters.h`

### S-004 DSP: Input Stage, Noise Gate, Boost
- Tier: Sonnet
- Profile: software
- Depends on: S-003
- Inputs: D-002 chain spec
- Actions:
  1. `Source/DSP/NoiseGate.h`: downward expander, 10 ms attack / 100 ms release, RMS detector, hysteresis 6 dB; automatable threshold (smoothed per D-006).
  2. `Source/DSP/Boost.h`: TS-style — input highpass ~720 Hz, symmetric soft clip (`tanh`), output lowpass; boost amount blends dry/clipped.
  3. Unit-test each stage in isolation before wiring (test-first: write T-004/T-005 harness now, watch it fail red against the stub, then implement to green).
- Outputs: `Source/DSP/NoiseGate.h`, `Source/DSP/Boost.h`, test harness `tests/dsp/` wired into ctest.
- Evidence produced: T-004 (partial), T-005 (partial)
- Done when: T-004 and T-005 pass for the built portion (voicing = flat passthrough after these stages).
- Checkpoint: completed += S-004.
- On failure: standard fix-and-rerun; DSP numeric bugs → record a lesson if a check caught your own mistake (Rule 11).
- Gate: none
- Relevant decisions/claims: D-002, D-003, D-006
- Lessons applied: none
- Exclusive resources: none

### S-005 DSP: Preamp Stages
- Tier: Opus
- Profile: software
- Depends on: S-004
- Inputs: D-002, D-003, `Voicings.h` row format defined here
- Actions:
  1. `Source/DSP/OversampledStage.h`: template wrapping `dsp::Oversampling<float>` (4x, polyphase) around any per-sample processor; Eco mode bypasses oversampling.
  2. `Source/DSP/Preamp.h`: two cascaded stages; each = pre-emphasis shelf → waveshaper (parameterized `tanh`/`x-a·x³` blend) → post-emphasis shelf; coefficients from `Voicings.h`.
  3. `Source/DSP/Voicings.h`: full 7-row table (A-005) — gains, shelf frequencies, shaper drive per stage.
  4. Extend T-005/T-006 harness to all voicings.
- Outputs: the three headers + voicing table.
- Evidence produced: T-005, T-006
- Done when: T-005 passes all voicings × rates × modes; T-006 passes.
- Checkpoint: completed += S-005.
- On failure: if a voicing clips internally before the shaper (unexpected -inf/NaN), normalize stage gains in the voicing row, not the test (immutability 2E.3).
- Gate: none
- Relevant decisions/claims: D-002, D-003, C-006, C-007
- Lessons applied: none
- Exclusive resources: none

### S-006 DSP: Tone Stack
- Tier: Sonnet
- Profile: software
- Depends on: S-005
- Inputs: D-002
- Actions:
  1. `Source/DSP/ToneStack.h`: 3-band (bass/mid/treble) + presence + depth; per-voicing center frequencies (e.g. Tweed warmer, Frost scooped-mid voicing curve); implemented as cascaded biquads (`dsp::IIR`) with coefficient recalculation only on parameter change (block-rate max).
  2. Smooth band gains per D-006.
- Outputs: `Source/DSP/ToneStack.h`.
- Evidence produced: T-005, T-006 (updated baselines), T-010 (Treble leg)
- Done when: T-005, T-006, T-010 pass.
- Checkpoint: completed += S-006.
- On failure: coefficient instability at 192 kHz → switch the affected biquad to `dsp::SVF`; log decision in `DEVIATIONS.md` (reversible, non-frozen).
- Gate: none
- Relevant decisions/claims: D-002, D-006
- Lessons applied: none
- Exclusive resources: none

### S-007 DSP: Power Amp (Saturation + Sag)
- Tier: Opus
- Profile: software
- Depends on: S-006
- Inputs: D-003
- Actions:
  1. `Source/DSP/PowerAmp.h`: output-tube-style soft-knee saturator (asymmetric blend, per-voicing knee), plus sag: envelope follower (20 ms/150 ms) modulating shaper bias ±; depth control scales the effect.
  2. Resonance/depth low-end thump: low shelf driven by sag envelope.
- Outputs: `Source/DSP/PowerAmp.h`.
- Evidence produced: T-005, T-006 (final DSP baselines), T-007
- Done when: T-004–T-007 all pass for all voicings.
- Checkpoint: completed += S-007.
- On failure: sag pumping artifacts → lengthen release to 250 ms max, cap bias modulation to ±20%; document in `DEVIATIONS.md`.
- Gate: none
- Relevant decisions/claims: D-003
- Lessons applied: none
- Exclusive resources: none

### S-008 DSP: Cabinet IR Convolution
- Tier: Sonnet
- Profile: software
- Depends on: S-007
- Inputs: D-004
- Actions:
  1. `Source/DSP/CabIR.h`: `dsp::Convolution` (uniform partitioning, stereo, non-real-time loading); 8 factory IR slots + 1 user slot; mic position = equal-power crossfade of close/off-axis pair; IR mix = wet/dry.
  2. User IR loader: background thread, `AudioFormatManager`, resample to host rate via `dsp::ResamplingAudioSource`-style offline resample; atomic swap into the audio thread (double-buffered convolution state).
  3. Factory IR placeholder set (self-generated synthetic IRs OK for tests; real captures land in S-016). A dB-vs-log-frequency cabinet table can be converted to a minimum-phase FIR for these placeholders (K-20261004T162311Z).
- Outputs: `Source/DSP/CabIR.h`, `Source/DSP/UserIRLoader.{h,cpp}`.
- Evidence produced: T-012
- Done when: T-012 passes; T-004/T-005 still green with IR active.
- Checkpoint: completed += S-008.
- On failure: convolution thread-safety issue → use `dsp::Convolution`'s built-in `loadImpulseResponse` with `NonUniform` config instead of custom double-buffering; record lesson.
- Gate: none
- Relevant decisions/claims: D-004
- Lessons applied: none
- Exclusive resources: none

### S-009 Voicing Calibration (gain staging across the genre range)
- Tier: Opus
- Profile: software
- Depends on: S-008
- Inputs: A-005, A-010
- Actions:
  1. Calibrate each voicing row so that: (a) loudness-matched (±1.5 dB LUFS) across voicings at matched dial positions (per-preset loudness compensation table); (b) gain ordering is monotonic Tweed < Plexi < JCM800 < Recto < 5150 ≈ Chainsaw < Frost at identical Drive settings; (c) T-006 tolerances met.
  2. Render a 10 s DI reference clip through each voicing at defaults into `validation/renders/` (input: checked-in DI WAV from tests/bench).
- Outputs: calibrated `Voicings.h`, `validation/renders/voicing-*.wav`.
- Evidence produced: T-006 final, renders for G-010
- Done when: T-006 passes and the loudness spread check (script `tests/loudness_spread.py`) reports ≤ ±1.5 dB.
- Checkpoint: completed += S-009.
- On failure: adjust voicing rows only; if monotonicity is unreachable with the waveshaper family, change that voicing's stage count (2→3) as a D-002-conformant row change; log in `DEVIATIONS.md`.
- Gate: none
- Relevant decisions/claims: D-002, D-003
- Lessons applied: none
- Exclusive resources: `Source/DSP/Voicings.h`

### S-010 GUI
- Tier: Sonnet
- Profile: software
- Depends on: S-003 (may run concurrently with S-004–S-009)
- Inputs: A-011, D-005
- Actions:
  1. Custom `LookAndFeel_V4` subclass: dark flat theme, vector rotary knobs, amp-section layout (Input/Gate | Boost | Preamp | Tone | Power | Cab | Output), preset browser bar on top.
  2. Every control bound via `SliderAttachment`/`ButtonAttachment`/`ComboBoxAttachment` to its APVTS parameter (this is what makes gesture discipline T-011 natural). Choice parameters are `AudioParameterChoice` shown as **non-editable** combo boxes — ComboBoxAttachment spaces items linearly over the parameter range and typed non-item text has no representation (K-20261009T164200Z), and JUCE 8.0.15's ComboBox has no text-editor accessor (K-20261009T164300Z).
  3. Resizable editor (`setResizable(true, true)`, min 800×450, constrainer ratio-free).
  4. Oversampling mode selector with a tooltip stating the latency (T-051 values).
- Outputs: `Source/UI/` module.
- Evidence produced: T-011 (with S-012), T-020 editor tests
- Done when: pluginval editor tests pass; GUI renders at min size with all controls reachable (DR-5).
- Checkpoint: completed += S-010.
- On failure: DR-5.
- Gate: none
- Relevant decisions/claims: D-005, D-007, A-011
- Lessons applied: none
- Exclusive resources: none

### S-011 Preset System + 24 Factory Presets
- Tier: Sonnet
- Profile: software
- Depends on: S-009, S-010
- Inputs: D-008, A-006
- Actions:
  1. `Source/Presets/PresetManager.{h,cpp}`: factory list from BinaryData XML; user dir create-on-first-use; schema `version="1"` with tolerant parsing.
  2. Author the 24 factory presets (genre list in A-006) as APVTS XML snapshots — each preset references only frozen parameter IDs.
  3. Preset browser in the GUI bar: previous/next + dropdown.
- Outputs: `Source/Presets/`, `Resources/presets/*.xml`.
- Evidence produced: T-008, T-013
- Done when: T-008 and T-013 pass.
- Checkpoint: completed += S-011.
- On failure: a preset referencing a stale ID fails T-008 → fix the preset, never the ID.
- Gate: none
- Relevant decisions/claims: D-008
- Lessons applied: none
- Exclusive resources: `Resources/presets/`

### S-012 Automation Hardening
- Tier: Opus
- Profile: software
- Depends on: S-010, S-011
- Inputs: D-006, D-007, A-007
- Actions:
  1. Audit every audio-thread parameter read: raw atomic reads into smoothed values at block top; no `getRawParameterValue` derefs inside per-sample loops without smoothing.
  2. Verify gesture discipline code paths (attachments + preset manager) against T-011's mock.
  3. Verify choice parameters report correct stepped automation; oversampling choice applies at block boundary with a 5 ms crossfade to the new path.
  4. Re-run T-010, T-011, T-020.
- Outputs: audit notes appended to `validation/automation-audit.md`.
- Evidence produced: T-010, T-011 final
- Done when: T-010, T-011, T-020 green; audit file lists every parameter with its smoothing mode.
- Checkpoint: completed += S-012.
- On failure: DR-4 second-host check if a host-side defect is suspected.
- Gate: none
- Relevant decisions/claims: D-006, D-007, C-004, C-008
- Lessons applied: none
- Exclusive resources: none

### S-013 Performance & Oversampling Budget
- Tier: Sonnet
- Profile: software
- Depends on: S-012
- Inputs: A-010, DR-3
- Actions:
  1. Build `tests/bench/` harness (standalone, renders checked-in DI WAV, wall-clock per block).
  2. Measure all voicings × {Eco, High}; record results as a CI artifact `bench-results.json`.
  3. If over budget → apply DR-3 ladder in order; each rung re-runs T-050/T-051.
- Outputs: `tests/bench/`, `bench-results.json`.
- Evidence produced: T-050, T-051
- Done when: T-050 and T-051 pass.
- Checkpoint: completed += S-013.
- On failure: DR-3 ladder exhausted → halt with `BLOCKED.md` (target is load-bearing, A-010).
- Gate: none
- Relevant decisions/claims: A-010, C-007
- Lessons applied: none
- Exclusive resources: CI runner timing (never benchmark two steps concurrently on one runner)

### S-014 CI Validation & Freeze Wiring
- Tier: Haiku
- Profile: software
- Depends on: S-013
- Inputs: T-020, T-021, T-040, T-041, T-050
- Actions:
  1. Extend `.github/workflows/ci.yml`: pluginval strictness 5 on built VST3 (both OSes) + AU (macOS) using `Tracktion/pluginval/validate@v2`; Steinberg validator via pluginval integration; security greps (T-040/T-041); benchmark regression gate (T-050); freeze-manifest check. Prerequisite checks from R0 knowledge: pushing workflow files needs the credential's **Workflows** permission, separate from Contents — verify first, 403 otherwise (K-20261009T160122Z); a `workflow_dispatch` workflow shows no Run button until it exists on the default branch — validate via PR/push triggers on the impl branch (K-20261009T160121Z). If Actions logs/artifacts are unreachable from the implementer's sandbox, push a JSON diagnostics summary to a `ci-diag/*` branch readable via the Contents API (K-20261009T160120Z).
  2. Add release workflow skeleton: on `v*` tag → build all artifacts → draft release (publish only at G-002).
- Outputs: full CI/release workflows, green runs on the impl branch.
- Evidence produced: T-020, T-021, T-040, T-041 in CI
- Done when: CI green on both OSes including validation and security gates.
- Checkpoint: completed += S-014.
- On failure: DR-2, DR-7.
- Gate: none
- Relevant decisions/claims: C-003
- Lessons applied: none
- Exclusive resources: none

### S-015 Ableton Live Manual Test Protocol
- Tier: Opus (execution by human-capable implementer session on a machine with Live 12)
- Profile: software
- Depends on: S-014
- Inputs: T-030–T-034, `plan/GATES.md` G-010
- Actions:
  1. Execute T-030–T-034 verbatim on Windows and macOS; record each item in `validation/live-test-record.md` (Live version, OS, buffer, result, notes).
  2. Render one 10 s clip per factory preset through Live (DI loop → plugin → bounce) into `validation/renders/preset-*.wav`.
  3. Capture CPU meter screenshots at High oversampling.
  4. Assemble `GATE-G-010.md` and halt for tone sign-off.
- Outputs: `validation/live-test-record.md`, renders, screenshots, `GATE-G-010.md`.
- Evidence produced: T-030–T-034
- Done when: all T-03x items recorded pass, or DR-4 applied and evidence attached.
- Checkpoint: completed += S-015.
- On failure: DR-4.
- Gate: **G-010**
- Relevant decisions/claims: C-002, D-006, D-007
- Lessons applied: none
- Exclusive resources: the Live test machine

### S-016 Packaging, IR Provenance, Installers
- Tier: Sonnet
- Profile: software
- Depends on: S-015 (G-010 signed)
- Inputs: D-004, D-009, DR-6, T-060
- Actions:
  1. Replace synthetic placeholder IRs with final original/CC0 captures; write `Resources/ir/PROVENANCE.md` (source, license, date) for each of the 8 factory IRs — DR-6 applies to any IR without documented provenance.
  2. Windows: ZIP of the `.vst3` bundle + README install note (installer optional; unsigned per ENVIRONMENT) — note that installing to `C:\Program Files\Common Files\VST3\` requires elevated write access; any installer must request elevation (K-20261009T092103Z). macOS: `.dmg`/`.pkg` with VST3 + AU, unsigned-first, notarization optional per G-002 rescope.
  3. Run T-060 installer smoke test on both OSes; record in `validation/install-smoke.md`.
  4. Write `CHANGELOG.md` v1.0.0 entry and `README.md` user docs (install paths for Live: `C:\Program Files\Common Files\VST3\`, `/Library/Audio/Plug-Ins/VST3/`, rescan note).
- Outputs: `Resources/ir/` final + PROVENANCE.md, packaging scripts, smoke-test record.
- Evidence produced: T-060
- Done when: T-060 passes; provenance file covers every shipped IR.
- Checkpoint: completed += S-016.
- On failure: DR-6 for IRs; signing failures are descoped via G-002, not fixed ad hoc.
- Gate: none
- Relevant decisions/claims: D-004, D-009
- Lessons applied: none
- Exclusive resources: none

### S-017 Release Preparation
- Tier: Sonnet
- Profile: software
- Depends on: S-016
- Inputs: D-009, G-002
- Actions:
  1. Tag `v1.0.0` locally; push the tag → release workflow builds artifacts and creates a **draft** release.
  2. Verify draft contents: VST3 (Win/mac), AU (mac), Standalone, checksums, release notes.
  3. Assemble `GATE-G-002.md` evidence bundle and halt.
- Outputs: draft release URL, `GATE-G-002.md`.
- Evidence produced: none
- Done when: draft release complete with all artifacts; gate file written.
- Checkpoint: completed += S-017.
- On failure: DR-7; never publish without sign-off.
- Gate: **G-002**
- Relevant decisions/claims: D-009
- Lessons applied: none
- Exclusive resources: git remote (tag push)

### S-RETRO Retrospective
- Tier: Opus, fresh context
- Profile: software
- Depends on: S-017 (and every step before it)
- Inputs: `EXECUTION_LOG.md`, commits, `DEVIATIONS.md`, `BLOCKED.md`/`TEST_CHALLENGE.md` if any, `GATE-*.md`, CI history, `research/PRIOR_KNOWLEDGE.md`, lessons/knowledge on both branches
- Actions:
  1. (Haiku subagent) collect inputs into a single digest `retro-digest.md`. The retrospective re-runs nothing and changes no deliverable.
  2. Answer the four protocol questions: what went wrong and is every correction a lesson; what was overlooked (missing step, missing earlier check, rediscovered knowledge); which R0 lessons were not applied or recurred; which Subject facts lack knowledge items.
  3. Write `RETROSPECTIVE.md`; create an `L-` file per unrecorded error and a `K-` file per missing fact; every finding links to an ID.
- Outputs: `RETROSPECTIVE.md`, new lessons/knowledge.
- Evidence produced: none
- Done when: every finding links to an `L-`/`K-` file and the Appendix C schema check prints `all entries valid`.
- Checkpoint: completed += S-RETRO.
- On failure: fix the retrospective only; deliverables stay untouched.
- Gate: none
- Relevant decisions/claims: Rules 11–13
- Lessons applied: n/a
- Exclusive resources: none

### S-KNOW Lessons & Knowledge Push
- Tier: Haiku
- Profile: software
- Depends on: S-RETRO
- Inputs: `lessons/`, `knowledge/` (both branches), A-008
- Actions:
  1. Write `GATE-G-003.md` (IDs, titles, tags, schema-check output) and halt.
  2. On `push-to-proposed`/`push-to: <target>`: copy into the store's Appendix C layout, regenerate `INDEX.md` files, push (retry 3×; fallback `git bundle create knowledge-run-20261027T120000Z-guitar-amp-vst.bundle` and report path). If the store doesn't exist, create it with an empty `TAXONOMY.md` extended by this run's tags.
  3. `do-not-push` → done.
- Outputs: store push or bundle.
- Evidence produced: none
- Done when: push succeeded and indexes list every pushed ID, or `do-not-push`, or bundle fallback written and reported.
- Checkpoint: completed += S-KNOW; run closed.
- On failure: per 3.7.2 fallback ladder.
- Gate: **G-003**
- Relevant decisions/claims: A-008, Rules 11–13
- Lessons applied: n/a
- Exclusive resources: knowledge-store remote
