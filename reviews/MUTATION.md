# MUTATION — T-070 result and survivor justifications

**Run:** CI run 40898109414, job `mutation`, Mull 19 (`mull.yml`, mutators `cxx_default`).
**Score:** 120 of 154 mutants killed = **77.9 %** (floor 75 %). Enforced in CI by the
"Mutation score gate" step reading `mull.sqlite`; the report and log are uploaded as the
`mull-report` artifact.

**Scope:** `Source/DSP/*` and `Source/Presets/*` (see `mull.yml`). The job runs the fast subset
of the suite (D-001l) plus the added characterisation tests in `tests/MutationTests.cpp`;
`silence`, `denormals` and `fuzz_presets` are omitted for runtime, so mutants only they would
kill count as survivors.

| File | Killed | Score |
|---|---|---|
| ToneStack.h | 4/4 | 100 % |
| PowerAmp.h | 22/23 | 96 % |
| Boost.h | 8/9 | 89 % |
| Preamp.h | 13/15 | 87 % |
| NoiseGate.h | 28/34 | 82 % |
| PresetManager.cpp | 11/14 | 79 % |
| CabIR.h | 25/40 | 62 % |
| UserIRLoader.cpp | 8/13 | 62 % |
| OversampledStage.h | 1/2 | 50 % |

History: 37.7 % (first real run) → 68.2 % (reference tests for Boost, NoiseGate, ToneStack,
PowerAmp) → 77.9 % (CabIR, UserIRLoader, preset list). The first rounds also exposed a product
bug (D-001m: the noise gate never attenuated).

## Surviving mutants by class (34)

**A. Loop-bound off-by-one on a sample/channel/preset index (`i < n` → `i <= n`) — 5.**
Boost.h:50, CabIR.h:191, PresetManager.cpp:168 (`<=` and `>=`), :246. The extra iteration reads
or writes one element past the end (or, for the preset loops, a child index that does not
exist and is ignored by `hasType`). It is undefined behaviour that a functional test cannot
observe without a sanitizer. Justification: not observable by a functional test. Covered instead by the CI job `sanitize`
(ASan + UBSan over the fast subset and the T-013 fuzz corpus), which fails on an out-of-bounds
access; no sanitizer finding in run 40925278824.

**B. Boundary comparisons where the boundary value is a measure-zero input (`>` ↔ `>=`) — 14.**
NoiseGate.h:71, 78, 83, 90, 93; Preamp.h:68, 69; PowerAmp.h:62; CabIR.h:37, 54, 102, 138 (×2);
UserIRLoader.cpp:62, 69, 94. They differ only when a float equals the threshold exactly
(envelope == previous envelope, sample == 0, a knob exactly at a clamp edge, an IR of
exactly 0 samples). Floating-point signals do not hit these exactly, and the alternative
outcome is equal in behaviour at the boundary (e.g. `a > env` vs `a >= env` select the same
coefficient when `a == env` because the follower output is unchanged). Equivalent for
practical purposes.

**C. JUCE default state makes the comparison constant — 3.**
CabIR.h:138 `getCurrentIRSize() > 0` (two mutants) and the readiness checks: a freshly
prepared `dsp::Convolution` already reports a one-sample IR, so `> 0`, `>= 0` and the guarded
branch cannot be told apart from outside. CabIR.h:94 `pendingFactory != currentFactory` loop
bounds (`engine < 2`, `<=`, `>=`) are the two-engine loop; the third iteration looks up a
non-existent name and is skipped (`data == nullptr`).

**D. Equivalent arithmetic — 4.**
NoiseGate.h:45 `winLen` is computed but unused (flagged in D-001m for the maintainers; the
design text says 5 ms, the implementation uses a 4096-sample box). OversampledStage.h:68
`baseRate * 4.0` is passed to the callback as the oversampled rate and only used by stages that
do not depend on it in the test configurations. UserIRLoader.cpp:74 `srcRate - rate` →
`srcRate + rate` takes the resampling branch for same-rate files; the resampler at ratio 1
reproduces the signal within the tolerance used by the loader tests. CabIR.h:162 `m <= 0` →
`m < 0`: at mix 0 the processed path produces `dry * d + 0 * wet`, bit-identical to the early
return.

**E. State-machine branches reachable only with threaded timing — 8.**
CabIR.h:96, 102 (`ne`→`eq`), 104, 94 (`lt_to_ge`) and CabIR.h:37 `<`/`>`: the
first-prepare/"keep last factory" fallbacks depend on whether the JUCE background loader has
finished; their observable difference is a delay of a few blocks before a cab swap becomes
audible, which the readiness pump in the tests hides. Equivalent after the engines settle.

Counts overlap by construction (a mutant is listed in the class that explains it); the table
above is the authoritative per-file tally.

## Reproduction

```
gh run download 40898109414 -n mull-report   # mull.sqlite + mull.log
python3 - <<'PY'
import sqlite3
rows = sqlite3.connect('mull.sqlite').execute('select execution_status from mutant').fetchall()
print(sum(1 for (s,) in rows if s != 2), '/', len(rows))   # status 2 = survived
PY
```

## 2026-11-06 - first runs after the D-001x to D-001ae preamp changes: no score

| Run | Commit | Result |
|---|---|---|
| 42091872467 | 9db05cc | Mull installed and built (42 min), mutants ran 198+ minutes, cancelled by the 240-minute job limit |
| 42128245981 | 377684e | Mull installed and built (3 min), mutants ran 3 h 40 min (23:19 to 02:59 UTC), then **"The hosted runner lost communication with the server"**; the job log and the report upload were lost |

There is no T-070 score for the current `Source/DSP/Preamp.h` and no per-file breakdown. The last recorded score (77.9 %, run 40898109414) predates the changes and does not describe the current code.

Cause of the second failure is not proven. The runner loss is the signature of memory or CPU starvation, and the mutated file gained per-sample state and loops (D-001aa, D-001ab, D-001ae) with four mutant workers on a 4-vCPU runner. A mutant that runs away or allocates without limit would explain it. The first run's long duration points the same way.

Proposed next step (not done):
1. Shard by file: one job per mutated file (`Preamp.h`, `PowerAmp.h`, `ToneStack.h`, `CabIR.h`, `NoiseGate.h`, `Boost.h`, `PostEq.h`, `UserIRLoader.cpp`, `Presets/`), each with `mull_include_paths.py` limited to its file, a 120-minute timeout and `--workers 3`.
2. Cap every test process (`ulimit -v` about 2 GB, `ulimit -t` about 120 s) so a runaway mutant dies alone.
3. Upload `mull.sqlite` and `mull.log` with `if: always()` so a lost runner still leaves partial results, and combine the shard scores with `tools/ci/mull_score.py`.

## 2026-11-06 - sharded full run on bd50813 (run 42196121659): gate passes, Preamp.h is below its baseline

Eleven shards, one per mutated file (D-001ak); all succeeded and `mutation-gate` passed.

```
Source/DSP/OversampledStage.h        1/2     50.0%
Source/DSP/Preamp.h                 28/50    56.0%
Source/DSP/CabIR.h                  24/40    60.0%
Source/DSP/UserIRLoader.cpp         16/26    61.5%
Source/Presets/PresetManager.cpp    11/14    78.6%
Source/DSP/NoiseGate.h              29/36    80.6%
Source/DSP/PowerAmp.h               26/30    86.7%
Source/DSP/Boost.h                   8/9     88.9%
Source/DSP/BiquadDesign.h           57/58    98.3%
Source/DSP/PostEq.h                  2/2    100.0%
Source/DSP/ToneStack.h               4/4    100.0%
Mutation score: 76.0% (206/271 killed); floor 75%
```

Project score 76.0 % (206 of 271 mutants killed), floor 75 %, 11 percentage points of files below their baseline only in `Preamp.h`: 56.0 % (28/50) against a recorded 81.2 %. All other files are within the 5-point tolerance of `reviews/mutation-baseline.json` (CabIR.h 60.0 against 62.5, PowerAmp.h 86.7 against 90.0). The project gate passes only because the other files carry the total; an incremental run that touches `Preamp.h` would fail its per-file gate (required 76.2 %). The baseline file was not changed.

### Preamp.h survivors (22 of 50), all in code added by D-001x to D-001ae

| Lines | Code | Survivors | Class |
|---|---|---|---|
| 51, 53 | `fastTanh` polynomial and division | 2 | the approximation is only checked loosely; a wrong coefficient changes it by less than the tests' tolerances |
| 143, 144 | high-band path: `z = gain * drive^2 * (x - hiLp)`, dead-zone subtraction | 4 | no test drives the stage hard enough for the high-band path to matter |
| 150, 152, 159 | bias envelope comparison, the 8-sample refresh counter, the DC-block subtraction | 5 | envelope direction, refresh cadence and the DC block are not asserted |
| 172, 178 | `tanhBlend >= 0.999` shortcut and the blend `1 - tanhBlend` | 3 | the Frost shortcut and the cubic blend are not compared with a reference |
| 183, 184, 188 | `exp(-1/(tau*sr))` and the high-band corner coefficient | 8 | time constants and the 1.2 kHz corner are not measured |

By mutator: sub to add 7, div to mul 4, mul to div 4, add to sub 1, and one each of gt to le, gt to ge, post-inc to post-dec, eq to ne, ge to gt, ge to lt.

Unlike the survivors recorded earlier (loop bounds, hysteresis edges), these are not equivalent mutants: each changes the sound or the cost, and a reference test would kill them. Proposed tests, none written yet: (1) `fastTanh` against `std::tanh` on a grid (bound 1e-4); (2) a stage test with a high-level 3 kHz tone checking that the high-band term raises the 3 kHz level and that a 100 Hz tone gets almost none; (3) an envelope test: a burst then silence, checking attack (1 ms) and release (50 ms) times and that the output has no DC after the DC block; (4) Frost (blend 1.0) against the shaper with an explicit cubic of zero, and a blend of 0.5 against the formula; (5) a frequency-response check of the high-band split at 1.2 kHz.

## 2026-11-07 - new test `preamp_reference` (ctest 32/32) and local mutant check

`tests/MutationTests.cpp` gains `preamp_reference`, registered in `tests/CMakeLists.txt` and added to the Mull test list in the workflow. It restates the stage independently (bias, high band, emphasis shelves, DC block, time constants, all constants written out as literals) and compares a loud two-tone burst followed by silence, for three voicings, both stages and two drive settings, within 1e-3; it also checks that the test signal makes the high band and the dynamic bias matter, that no DC is left, that silence stays silent and that `fastTanh` is within 1e-4 of `tanh`. The stage matched its documented chain on the first run.

Twelve of the 22 survivors were re-created by hand and every one is now killed (`fastTanh` coefficient and division; envelope attack and release formulas; the high-band corner; the DC block; the 8-sample refresh condition; the blend term; the attack/release comparison; the high-band gain and dead-zone arithmetic; the Frost shortcut). The Mull result for the remaining ones comes from the next sharded run.

Survivors expected to stay, equivalent mutants:

- `a > env` to `a >= env` (the attack/release choice): the two branches differ only when `a == env`, where `a - env` is zero.
- `biasTick++` to `biasTick--`: the refresh still happens every eighth sample, starting with the first, so the output is identical.
- `tanhBlend >= 0.999f` to `> 0.999f`: no voicing has a blend of exactly 0.999.
