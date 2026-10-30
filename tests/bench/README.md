# Bench gates (T-050 / T-051)

`grimamp_bench --bench` writes `bench-results.json`. In CI, `tools/ci/check_bench.py` applies:

| Gate | Where | Enforced on |
|---|---|---|
| T-051 latency (Eco 0 samples, High <= 144, reported vs measured within +/-1) | `latency.*.passes` | every runner |
| T-050 budget: worst case `Frost/High` <= 15 % of one core | `bench-targets.json` | `windows-latest` (enforced); `macos-latest` reported only, relaxed by D-001n |
| T-050 regression: any voicing/mode > baseline x 1.20 and > 2 percentage points above it | `baseline-T050.json` | `windows-latest` (enforced); `macos-latest` reported only |

Post-conditions: the results file must exist, be non-empty, parse, and contain all 7 voicings x 2
modes and both latency entries; otherwise the step fails.

## Baseline bootstrap and refresh

`baseline-T050.json` holds per-runner-class CPU % taken from the artifacts of CI run 40898109414.
If a runner class has no baseline entry the check prints `BASELINE RECORDED`, passes, and still
uploads `bench-baseline-candidate.json` (artifact `bench-results-<runner>`). To refresh a baseline,
copy that candidate's `cpu_percent` block into `baseline-T050.json` under the runner label and commit
with a `DEVIATIONS.md` entry; a refresh that raises numbers is a maintainer decision (human gate),
never a way to make a regression pass.

## Known variance

Hosted runners vary run to run by more than the 20 % tolerance (see knowledge item
K-20261029T030800Z): a failure of the regression or budget gate is investigated, not silenced. If it
recurs, the maintainer chooses between a median-of-N bench, a wider tolerance or a pinned runner.
