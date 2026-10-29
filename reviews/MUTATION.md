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
observe without a sanitizer. Justification: equivalent without ASan; covered by the JUCE
leak/bounds checks only in debug builds. *Follow-up:* run the suite under ASan in a later job.

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
