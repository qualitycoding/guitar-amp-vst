# Protocol Conformance Review — planning-protocol v3.6

**Run:** `run-20261027T120000Z-guitar-amp-vst`
**Reviewed against:** `qualitycoding/skills` `protocols/planning-protocol.md` + `protocols/planning-protocol-review.md`, **v3.6**
**Plan written under:** v3.4 (amended 2026-10-27 from v3.3)
**Remote base for this amendment:** `impl-run-20261027T120000Z-guitar-amp-vst` @ `2fce35abd91558f62170f0f3a8c669d9a4fd36b0`
**Date of this review:** 2026-10-31
**Amendment deviation IDs:** D-001o (plan/protocol conformance), D-001p (CI Rule 14 hardening)

---

## 1. What changed between v3.4 (plan) and v3.6 (current)

| Protocol version | Additions |
|---|---|
| v3.5 | Rule 14 (honest verification: no swallowed failures, artifact post-conditions, budgets gate or are human-relaxed with a recorded decision, stored regression baselines, per-environment targets, substitutions logged as deviations); §1.3.6 Command & Pin Verification (pins must exist; commands spike-run against pinned versions; config files part of the pin); §3.7.1 retro question 1a (completeness diff of workflows/manifests vs plan/ENVIRONMENT.md) |
| v3.6 | §0.1 environment-class probe (`stable` vs `ephemeral`, evidence-based classification); §1.5 Ephemeral-Environment Protections (durable surface, turn-start restaging, exec-bit restoration, checkpointed resumable builds, ordering invariants, right-sizing steps to container lifetimes, CI for long work, blocked dimensions named CI-side never reported as pass); Rule 0.3 `[ephemeral]` environment tag; Global Rule 6 push cadence (commit+push after every successful chunk on ALL platforms, `ls-remote` verification, backoff retries, mismatch reporting); §3.4 resiliency hook; §3.5 HANDOFF restatement of Rule 6 |

The v3.5 review record explicitly cites this run as its source (green CI that
verified nothing via a `|| true` mutation; T-050 budget failing by >2x reported
as informational; no stored regression baseline; a `pluginval 2.0.0` pin that
does not exist; substitutions never logged). The v3.6 review record cites the
sonic-destruction-vst run (16+ container recycles; ASan build never completed).

## 2. v3.5 items — status

| Item | Status | Evidence |
|---|---|---|
| Substitutions logged as deviations | **DONE** | D-001 (JUCE re-pin), D-001a–D-001h (implementation), D-001i (pluginval → v2.0.0-beta3), D-001j (CI environment additions), D-001k (program name bug), D-001l (Mull fast subset), D-001m (gate bug), D-001n (macOS T-050 human decision) |
| Mutation job must be able to fail | **DONE** | `mutation` job enforces the 75 % floor from the SQLite report (`sys.exit(0 if score >= 75.0 else 1)`); score 77.9 % on run 40771992301; `reviews/MUTATION.md` committed |
| Budget check gates or is human-relaxed with a recorded decision | **DONE (human-relaxed, recorded)** | D-001n: maintainer decision 2026-10-30 keeps DSP and budget unchanged; macOS virtual-M1 figures (18.0–35.6 % vs 15 %) recorded as a known miss; bench step stays informational |
| Regression baseline for T-050 | **OPEN — residual, named** | No committed baseline. Recipe recorded in §5 below; deliberately NOT bootstrapped in this amendment because D-001n (the recorded human decision) keeps the step informational — re-gating belongs to the maintainer who signed D-001n. Recorded here so it is not silently absent. |
| Per-environment targets | **DONE (partially) / FIXED here** | D-001c named the sandbox as non-CI-class; D-001n recorded both runner classes' numbers. The per-class **target table** is added to `plan/ENVIRONMENT.md` by this amendment. |
| No swallowed failures in load-bearing steps | **FIXED here** (residual in remote ci.yml) | Remote ci.yml at 2fce35a still had: bench step `|| true`; `dumpbin //imports … || true` (vacuous pass — a missing dumpbin leaves `imports.txt` absent and `! grep` trivially succeeds); artifact uploads `if-no-files-found: ignore`. All three fixed by `.github/workflows/ci.yml.patch` (D-001p). |
| §1.3.6 pin verification | **FIXED here** | Plan pins were never re-verified after adoption: ENVIRONMENT.md still pinned pluginval **2.0.0**, which does not exist (only 2.x betas — D-001i), and still pinned JUCE 8.0.15 after D-001 adopted 8.0.16-security, with a setup command asserting `git -C JUCE describe --tags` prints 8.0.15 — a fresh implementer following the plan verbatim would fail S-000/S-001. Pin table and setup commands corrected in the amended `plan/ENVIRONMENT.md` (existence-check evidence cited per row). |
| Retro 1a completeness diff | **DONE here** | §4 below. |

## 3. v3.6 items — status

| Item | Status | Notes |
|---|---|---|
| §0.1 environment-class probe | **MISSING → FIXED here** | Never recorded. Probe result now in amended `plan/ENVIRONMENT.md` §0.1: **ephemeral**, with first-hand observed evidence (dated). Also recorded in `plan/PROFILE.md` (Rule 0.3 tag) and proposed for `.checkpoints/state.json`. |
| Rule 0.3 `[ephemeral]` tag | **MISSING → FIXED here** | Added to `plan/PROFILE.md`; consequence (§1.5 protections active) stated. |
| §1.5 Ephemeral-Environment Protections | **MISSING → FIXED here** | `tools/restage-env.sh` created (turn-start restaging from the durable surface); `plan/ENVIRONMENT.md` §1.5 records the durable-surface rule, exec-bit restoration, checkpointed resumable builds, the ordering invariants learned first-hand (CMake silently ignores env-var dependency overrides at configure time — pass dependency paths as `-D` cache args; configure re-touches generated headers, invalidating objects; the durable mount serves stale bytes to compilers and does not update mtimes on `touch` — delete objects to force rebuild and verify sources by content, not timestamp), right-sizing (LTO link ≈ 13–20 min exceeds a tool-call boundary → `setsid`-detached builds with log polling; `nohup` dies with the call), and CI for long work (already practiced: sanitize + mutation jobs). |
| Global Rule 6 push cadence | **ENVIRONMENT-BLOCKED, named fallback recorded** | The build sandbox has no outbound network (curl to github.com times out, exit 124 — verified repeatedly 2026-10-28…31). Push cadence therefore runs through the plan's own fallback ladder (S-KNOW 3.7.2 / D-001a): commits accumulate locally and ship as a **git bundle** handed to the human, who pushes and performs the `ls-remote` verification. Recorded in amended `HANDOFF.md` (Rule 6 restatement + fallback) and `plan/ENVIRONMENT.md` hosts table. Never reported as a pass locally — recorded as blocked with a named channel, per §1.5's honesty rule. |
| §3.4 resiliency hook | **RECORDED** | In use first-hand and now documented: git `index.lock` transient write failures (rm + bounded retry loop succeeded on attempt 3); transient cross-filesystem copy failures (verify by content after copy). |
| §3.5 HANDOFF Rule 6 restatement | **MISSING → FIXED here** | Amended `HANDOFF.md` restates Global Rule 6 verbatim-in-substance and records the bundle fallback. (HANDOFF also restated Rules 9/11/12/13 already.) |

Additional finding folded into the amendment: `HANDOFF.md`'s environment line said
"Pinned: JUCE 8.0.12" — stale against both the plan table (8.0.15) and D-001
(8.0.16-security). Corrected.

Documentation note (not an error): `plan/PLAN.md` S-000 action 3 says "Re-run
the 0.1 checks from this environment" meaning v3.4's 0.1 (access verification).
Under v3.6, §0.1 is the environment-class probe and the access checks moved.
PLAN.md is left untouched as a planning artifact; the mapping is recorded here
and in the amended ENVIRONMENT.md.

## 4. Retro 1a completeness diff (v3.5 §3.7.1)

Workflows and manifests at remote HEAD `2fce35a` diffed against
`plan/ENVIRONMENT.md` + `plan/PLAN.md` as written:

- **ci.yml** uses `Tracktion/pluginval/validate@v2.0.0-beta3` — a substitution
  against the plan text (`@v2`, pluginval 2.0.0) → logged as **D-001i** ✔
- **ci.yml** Mull job installs `mull-19`/`clang-19`/`lld-19` + JUCE Linux dev
  packages; per-test timeout 180 s; `.gitattributes` eol=lf — environment
  additions → logged as **D-001j** ✔
- **ci.yml** bench re-run step and artifact uploads — post-plan additions not
  reflected in the plan's pin/manifest tables; Rule 14 defects within them fixed
  by this amendment → **D-001p** ✔
- **mull.yml** — new config file, part of the Mull pin under §1.3.6 ("config
  files are part of the pin"); its existence is now noted in the amended
  ENVIRONMENT.md Mull row ✔
- **`.gitattributes`** — logged in D-001j ✔
- **Plan-side leftovers found by the diff:** pluginval 2.0.0 pin (nonexistent),
  JUCE 8.0.15 pin + `describe --tags` assertion contradicted by D-001,
  HANDOFF "8.0.12" line — all corrected by this amendment (D-001o) ✔
- **ASan job, reference tests, product fixes** (getProgramName, noise gate) —
  no plan/manifest contradiction; D-001k/D-001m cover the product side ✔

No unlogged substitution remains.

## 5. Residual items (named, not silently absent)

1. **T-050 stored regression baseline.** Recipe: after the maintainer re-gates
   the bench step (amending D-001n or superseding it): first CI run writes
   `tests/bench/baseline-T050.json` (per-runner-class CPU numbers from
   `bench-results.json`); a later step fails if any voicing exceeds
   `baseline × 1.20` (the plan's ">20 % vs main" regression rule) or the §1.3
   budget where the runner class meets it. Until then the step stays
   informational per D-001n and this absence is recorded here.
2. **macOS T-050 miss** (18.0–35.6 % vs 15 % on virtual Apple M1): recorded as
   a known miss by D-001n; Windows EPYC class passes (10.8–11.7 %). The honest
   paths forward are a dedicated perf runner or a budget relaxation by the same
   human-gate mechanism that produced D-001n — not an ad-hoc CI edit.
3. **`ls-remote` verification of pushes** cannot run inside the sandbox; it is
   the human-side postcondition of the bundle fallback (Rule 6), recorded in
   HANDOFF.md.
4. **PLAN.md S-000 "0.1 checks" wording** refers to the v3.4 protocol section
   numbering; see §3 documentation note.

## 6. Files in this amendment

| File | Action |
|---|---|
| `plan/ENVIRONMENT.md` | full replacement (§0.1 probe, corrected pins + setup commands, §1.5 protections, per-runner-class T-050 target table, hosts-table fallback row) |
| `plan/PROFILE.md` | full replacement (adds Rule 0.3 `[ephemeral]` environment tag) |
| `HANDOFF.md` | full replacement (corrected pins; Global Rule 6 restatement + bundle fallback) |
| `tools/restage-env.sh` | new — turn-start restaging from the durable surface (chmod +x on apply) |
| `.github/workflows/ci.yml.patch` | apply with `git apply` — removes `|| true` from the bench step and the dumpbin vacuous pass; artifact uploads become `if-no-files-found: error` |
| `DEVIATIONS.append.md` | append to `DEVIATIONS.md` — D-001o, D-001p |
| `EXECUTION_LOG.append.md` | append to `EXECUTION_LOG.md` |
| `lessons/L-20261031T000000Z-probe-environment-class-at-run-start.md` | candidate lesson (dedupe on apply) |
| `knowledge/K-20261031T000000Z-ephemeral-sandbox-invariants.md` | candidate knowledge item (dedupe on apply) |

See `APPLY.md` in this package for the exact application procedure.
