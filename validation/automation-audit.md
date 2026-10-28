# Automation Audit (S-012) — run-20261027T120000Z-guitar-amp-vst

Scope: how every parameter interacts with the audio thread, with automation
armed, per D-005/D-006/D-007 and the T-010/T-011 frozen tests.

## Parameter inventory (21 automatable parameters)

| ID | Type | Audio-thread handling |
|---|---|---|
| `input-gain` | float dB −24..24 | per-block `LinearSmoothedValue` ramp (20 ms) |
| `gate-on` | bool | raw atomic read at block top; gate skipped when off |
| `gate-threshold` | float dB −80..0 | per-block smoothed |
| `boost-on` | bool | raw atomic read at block top |
| `boost-amount` | float 0..1 | per-block smoothed |
| `drive` | float 0..1 | per-block smoothed |
| `bass` / `mid` / `treble` / `presence` / `depth` | float 0..1 | per-block smoothed (block-rate coefficient update; per-sample zipper bound verified by T-010) |
| `sag` | float 0..1 | per-block smoothed |
| `amp-voicing` | choice 0..6 | **stepped**: applied at block boundary, 5 ms crossfade |
| `cab-ir` | choice 0..8 (incl. user slot) | **stepped**: applied at block boundary, wait-free engine handoff, 5 ms crossfade |
| `oversampling` | choice Eco/High | **stepped**: re-prepares oversampled stage at block boundary, latency updated via `setLatencySamples` |
| `mic-position` | float 0..1 | per-block smoothed (close↔off-axis gain crossfade) |
| `ir-mix` | float 0..1 | per-block smoothed (wet/dry crossfade) |
| `post-eq-on` | bool | stepped at block boundary with 5 ms crossfade |
| `post-eq-low` / `post-eq-high` | float dB −12..12 | per-block smoothed |
| `master` | float 0..1 | per-sample smoothed (post-fader, most audible) |

## Findings

1. **Atomic reads.** Every parameter value is read via
   `apvts.getRawParameterValue(id)->load()` at block top (`pullSmoothParams` +
   `applySteppedParams`) — no locks, no allocations on the audio path.
2. **Stepped changes are click-guarded.** Voicing/cab/oversampling/post-eq-on
   switch only at block boundaries and trigger a 5 ms equal-power-style linear
   crossfade (`crossfadeSamplesLeft`). T-011 (gesture discipline) passes.
3. **Choice decoding is range-correct.** Choice parameters store their index
   normalised as `index/(count−1)`; the processor converts back with
   `convertFrom0to1` + round (`choiceIndex()`). A previous `(int)`-cast
   truncation silently mapped all voicings to Tweed and made the user-IR slot
   unreachable — caught by T-012 leg (a); fixed and covered by the frozen suite.
4. **Preset changes create no gestures (D-007).** `PresetManager::loadPreset`
   writes parameter values through the APVTS without
   `beginChangeGesture`/`endChangeGesture`, so loading a preset while automation
   is armed creates no automation lanes (frozen: T-032 in Live; locally the same
   code path is exercised by T-008 preset round-trip).
5. **Audio-thread IR swap is wait-free** (per JUCE docs,
   `Convolution::loadImpulseResponse` queues to a background thread); the
   processor hands the loaded buffer off at block boundary
   (`userIrLoader.takeLoadedBuffer`). Note for T-031: the new IR becomes
   audible a few milliseconds after selection on first load (JUCE background
   engine build); the UI reports the user slot as applied only after the
   handoff (`isUserIrApplied()`).
6. **Zipper bound.** T-010 (fastest legal automation sweep) passes; smoothed
   params ramp over 20 ms, stepped params crossfade over 5 ms.

## Machine-checked evidence

- `ctest` targets: `param_inventory`, `param_ranges`, `smoothing`, `gestures`,
  `preset_roundtrip`, `state_restore`, `sine_sanity`, `silence` — all green.
- T-031/T-032 (Live automation lanes) are human-gated (see BLOCKED.md B-3).
