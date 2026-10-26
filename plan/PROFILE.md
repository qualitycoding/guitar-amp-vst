# PROFILE — Active Profiles & Applicability

## Active profiles

| Profile | Active | Justification |
|---|---|---|
| `software` | **yes** | Producing a shippable, maintained C++ audio plugin. |
| `math` | no | No mathematical statements or proofs are deliverables. DSP transfer functions are engineering parameters, not theorems. |
| `computational` | no | Correctness is validated by tests/listening, not by numerical results whose correctness depends on computation. (Implies `software` only when active.) |
| `publication` | no | No manuscript, preprint, or archived scholarly record. |

## Mode flags

| Flag | Value | Justification |
|---|---|---|
| `math.exploration` | false | N/A (math profile inactive). |
| `software.deploys` | **true** | The plan includes packaging installers and a public release of a distributed artifact (installers/binaries) → gate `G-002` and `plan/OPERATIONS.md` are required. |

## N/A sections (produce no artifacts, impose no checks)

- 0.3.3 Mathematical intake; 0.3.4 Computational intake; 0.3.5 Publication intake
- 2A (all math research/specification sub-phases, incl. 2A.0 Exploration), 2C Figure Specifications, 2D Manuscript Specification
- `research/NOVELTY.md`, `math/` (all), `figures/`, `manuscript/`
- Rule 7 evidence classes; Rule 8 result-agnostic planning (math/computational portions)
- Gate `G-001` (math/computational headline result) — **not required**
- Phase 4 lenses: math correctness, computational numerics/reproducibility, novelty/prior-art, referee/venue lenses
- Phase 5 consistency checks for `M-###`, `F-###`, manuscript claims

## Required gates for this profile set (Rule 10)

- `G-002` — required (public release of installers/binaries).
- `G-003` — required (always).
- `G-010` — added as risk mitigation (Phase 4.4): human tone sign-off.

## Profile changes

None. Recorded in `.checkpoints/state.json` under `profiles` and `mode_flags`.
