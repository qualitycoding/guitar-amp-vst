# guitar-amp-vst (GrimAmp)

A rock and metal guitar amp simulator plugin (VST3, AU on macOS, and Standalone), working title
**GrimAmp**. It covers 70s blues rock through to high-gain black metal, with seven amp voicings on one
engine, cabinet impulse responses and factory presets.

> **Status: implemented on a separate branch, not released.** This default branch
> (`gen-20261027T120000Z-guitar-amp-vst`) holds the plan, research, pre-mortem, review notes and the
> frozen test specification. The plugin code is on the **`impl-run-20261027T120000Z-guitar-amp-vst`**
> branch, which has the full source, tests and a more detailed README. It has not been merged.

## What the implementation branch contains

- Seven voicings: Tweed, Plexi, JCM800, Recto, 5150, Chainsaw and Frost.
- A signal chain with a TS-style boost, noise gate, tone stack (bass, mid, treble, presence, depth),
  power-amp saturation with sag, and cabinet convolution with a close/off-axis mic crossfade. Eco
  (zero latency) and High (4x oversampled) modes.
- 8 factory cabinet impulse responses plus a user IR slot, and 24 factory presets. The factory IRs are
  synthetic CC0 placeholders (`tools/gen_ir.py`), not real cabinet captures.
- Every parameter is host-automatable.
- A frozen test suite (13 unit and DSP test targets, preset fuzzing, loudness and benchmark tools),
  pluginval at strictness 5, and mutation testing with Mull.
- Licence: AGPL-3.0, using JUCE under its AGPL option.

## What is not done

- Sound quality has not been judged by ear. The blind listening gate (G-010) is blocked until final
  original or CC0 cabinet IRs are chosen and candidate renders are refreshed.
- Validation on Windows and macOS, and testing in Ableton Live 12, were still open items when the
  branch's `BLOCKED.md` was written. Check `EXECUTION_LOG.md` on that branch for the latest CI results.
- No release has been made. The changelog calls v1.0.0 a draft pending gate G-002.

## Building

Build from the implementation branch, not from this one:

```bash
git checkout impl-run-20261027T120000Z-guitar-amp-vst
git submodule update --init --recursive
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target GrimAmp_VST3
ctest --test-dir build --output-on-failure -C Release
```

## Notes on this branch

- Start with `plan/PLAN.md`, `plan/DECISIONS.md` and `tests/TEST_SPEC.md`.
- `HANDOFF.md` on this branch is corrupted: it is a raw GitHub API response (a base64 blob) followed by
  text. The readable version is on the implementation branch.
- `scripts/make_blind_pairs.py` and `scripts/score_blind.py` build and score the blind listening test.
