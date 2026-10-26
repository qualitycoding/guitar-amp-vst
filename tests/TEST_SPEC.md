# FROZEN — DO NOT MODIFY
# TEST_SPEC — Frozen Tests & Checks (T-###)
# Freeze protocol: Phase 2E. Hashes in FROZEN_MANIFEST.sha256. Immutability rule 2E.3 applies.
# Implementers cannot modify, skip, mark expected-failure, or weaken these tests.
# If a frozen test is found invalid: halt, write TEST_CHALLENGE.md (Phase 2E.4).

Categories: unit (juce::UnitTest via ctest), dsp (golden/numeric), host (pluginval), live (manual Ableton protocol), security, performance, operational.

---

## Unit & DSP tests (automated, ctest)

### T-001 Skeleton instantiation
The plugin constructs, calls `prepareToPlay(44100, 512)` and `processBlock` with silence, and releases — without crash or leak (JUCE leak detector clean). **Done when:** ctest `skeleton` passes. *(Serves S-002.)*

### T-002 Parameter inventory
`getParameters().size()` equals the frozen count in `Source/Parameters.h` (target ≈ 30, exact count frozen when S-003 lands — the test reads the count from a checked-in `tests/parameter_ids.txt`); every parameter ID in that file exists, is unique, and matches `^[a-z0-9-]+$`. **Done when:** ctest `param_inventory` passes.

### T-003 Parameter defaults & ranges
Every float parameter's default lies within its NormalisableRange; every choice parameter's default index < choices size. **Done when:** ctest `param_ranges` passes.

### T-004 Silence in → silence out
With input digital silence for 10 s at 48 kHz, output RMS < -120 dBFS for every voicing (gate open or closed). **Done when:** ctest `silence` passes.

### T-005 Sine sanity / no NaN or Inf
1 kHz sine at -12 dBFS through every voicing × {44.1, 48, 96, 192 kHz} × {Eco, High oversampling}: output contains no NaN/Inf, peak < 0 dBFS + 1 dB tolerance, and is not silent (RMS > -60 dBFS). **Done when:** ctest `sine_sanity` passes.

### T-006 Harmonic structure per voicing
1 kHz sine at -20 dBFS, gain at 50%: FFT of output shows 2nd and 3rd harmonics above the noise floor (> -80 dBFS) for high-gain voicings (5150, Frost, Chainsaw), and THD of `Tweed` at gain 20% < THD of `Frost` at gain 80% (monotonicity of gain staging across voicings). Tolerances frozen as constants in the test. **Done when:** ctest `harmonics` passes.

### T-007 Denormal immunity
Feed 1e-20 amplitude DC offset mixed with 1 kHz sine for 60 s at 48 kHz: CPU time per block must not exceed 150% of the T-005 baseline median, and output contains no NaN. (FTZ/DAZ enabled via `ScopedNoDenormals` in processBlock.) **Done when:** ctest `denormals` passes.

### T-008 Preset round-trip
For every factory preset (A-006): load preset → `apvts.copyState()` → save to memory XML → `replaceState()` → `copyState()` again → the two XML trees are equal. Also: user-preset save→load round-trip via a temp dir. **Done when:** ctest `preset_roundtrip` passes (24 presets).

### T-009 State restore (DAW session recall)
Set every parameter to a pseudo-random in-range value (seed 1234), `getStateInformation` → new processor instance → `setStateInformation` → all parameter values match. **Done when:** ctest `state_restore` passes.

### T-010 Zipper-noise bound (automation smoothing)
Automate Master output from 0.0→1.0 over one 512-sample block (worst case): sample-to-sample delta of the rendered 1 kHz sine output never exceeds 3× the steady-state peak delta (no click transient). Repeat for Drive and Treble. **Done when:** ctest `smoothing` passes.

### T-011 Gesture discipline (host notification)
With a mock `AudioProcessorListener`: a GUI slider drag produces exactly one `beginChangeGesture` … `endChangeGesture` pair around ≥1 `setValueNotifyingHost`; a factory-preset load produces **zero** `setValueNotifyingHost` calls (values go through `replaceState`, D-007). **Done when:** ctest `gestures` passes.

### T-012 IR loading robustness
User-IR slot: (a) valid 48 kHz WAV loads and changes the output (differs from bypass RMS by > 3 dB); (b) 96 kHz WAV is resampled and loads; (c) truncated/corrupt WAV fails gracefully — plugin keeps processing with the previous IR, no crash, no NaN. **Done when:** ctest `ir_loading` passes.

### T-013 Fuzz: malformed preset files
1000 mutated preset XMLs (bit flips, truncations, huge numbers; generated with seed 42, checked-in generator script `tests/fuzz_preset.py`) — loading each never crashes, never throws uncaught, and the processor remains in a valid state (T-009 passes afterward). **Done when:** ctest `fuzz_presets` passes.

## Host validation

### T-020 pluginval strictness 5
`pluginval validate --strictness-level 5` exit code 0 for VST3 (Windows + macOS) and AU (macOS), run in CI on every PR and on release tags. Covers pluginval's basic, bus, editor, parameter-fuzz, locale, and extreme tests.

### T-021 Steinberg VST3 validator
JUCE 8.0.12's bundled VST3 SDK 3.8.0 validator (invoked through pluginval's integration) passes with 0 errors.

## Ableton Live manual protocol (human-executed, recorded in `validation/live-test-record.md`)

### T-030 Load & scan
Plugin installs to the system VST3 folder, Live 12 rescans without the plugin appearing in the crashed list, and the plugin loads on an audio track. *(Live 12 supports 64-bit VST3 on Win/mac — research claim C-002.)*

### T-031 Automation of every parameter
In Live: every parameter appears in the plugin's automation parameter list (device "Configure" / unfold), and recording automation for 6 representative parameters (Drive, Bass, Mid, Treble, Master, Amp Voicing choice) plays back correctly with no audible clicks (matches T-010).

### T-032 Preset change does not spawn automation
Loading factory presets from the plugin's browser while Live's automation-arm is on creates **no** new automation lanes (D-007).

### T-033 Session recall
Save a Live set with a non-default preset and 3 automated parameters, close, reopen: preset and automation restored exactly (T-009's host-level twin).

### T-034 Buffer/sample-rate sweep
Live at buffer sizes {64, 256, 1024} and sample rates {44.1, 96 kHz}: no dropouts, no crashes, audio passes on all.

## Security

### T-040 No network
Binary contains no socket/URL open calls: CI greps the built binaries' import/symbol tables (Windows `dumpbin /imports`, macOS `otool -L` + `nm`) against an allowlist; `JUCE_USE_CURL=0` is asserted at compile time.

### T-041 Local file handling only
Code search gate (CI): file I/O occurs only in the preset and user-IR modules (`Source/Presets/`, `Source/DSP/UserIR*`); everything else flagged.

## Performance

### T-050 CPU budget (A-010)
Standalone benchmark harness (`tests/bench/`) renders 60 s of a checked-in DI guitar WAV at 48 kHz/256 buffer, worst-case voicing (Frost, High oversampling, IR active): mean CPU ≤ 15% of one core on the CI runner class recorded in `plan/ENVIRONMENT.md`. Reported per-PR as a CI artifact; regression > 20% vs `main` fails the job.

### T-051 Latency report
`getLatencySamples()` == 0 in Eco mode and ≤ 144 samples (≈3 ms @ 48 k) in High mode; reported latency matches the measured impulse delay within ±1 sample for both modes.

## Operational

### T-060 Installer smoke test
Fresh user account (or clean VM snapshot): install → Live rescan → load → factory presets present → audio passes → uninstall leaves no running processes. Executed on both OSes before G-002; recorded in `validation/install-smoke.md`.
