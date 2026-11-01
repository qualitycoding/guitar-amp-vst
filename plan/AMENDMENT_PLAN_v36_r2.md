# AMENDMENT PLAN — protocol v3.6 conformance, round 2

**Run:** `run-20261027T120000Z-guitar-amp-vst` · **Date:** 2026-10-31
**Plan written under:** v3.4 (amended from v3.3) · **Reviewed against:** `qualitycoding/skills`
`protocols/planning-protocol.md` **v3.6** (HEAD `b361dab`) and `planning-protocol-review.md`
**Builds on:** round 1, commit `abdc2a9` (D-001o / D-001p, `reviews/PROTOCOL_CONFORMANCE_v36.md`).
Round 1 is not redone; this round verifies it, closes what it left open, and adds what an
independent read of the protocol found missing.

## 1. Delta v3.4 → v3.6 (what the protocol now requires)

| ID | Protocol item | Requirement in one line |
|---|---|---|
| P1 | Rule 14 no swallowed failures | No `\|\| true`, `continue-on-error`, `set +e` on load-bearing commands; flaky tools get bounded retries |
| P2 | Rule 14 artifact post-conditions | Every step's Done-when lists artifact existence **and** a sanity check; CI fails when an artifact is missing |
| P3 | Rule 14 budgets gate | A measurable acceptance criterion fails the build when missed; informational only for metrics with no criterion |
| P4 | Rule 14 baselines bootstrap | Regression criteria say where the first baseline is stored and what happens before it exists |
| P5 | Rule 14 per-environment targets | Each target names its runner class and has a pre-decided miss rule; relaxing is a human-gate decision |
| P6 | Rule 14 substitutions are deviations | Every substitution logged in `DEVIATIONS.md` when made; retro diff against `plan/ENVIRONMENT.md` |
| P7 | 1.3.6 command & pin verification | Pins exist (with hashes); literal commands spike-run or doc-verified against the pinned version; tool config is part of the pin |
| P8 | 3.6/5 consistency check | No swallowing idiom in plan or CI; every numeric-threshold `T-###` maps to a step whose failure fails the build |
| P9 | 3.7.1 retro question 1a | Completeness diff of workflows/manifests vs `plan/ENVIRONMENT.md` |
| P10 | 0.1 environment class + `[ephemeral]` tag + 1.5 | Probe recorded in `.checkpoints/state.json`; protections apply iff ephemeral |
| P11 | Rule 6 push cadence | Push after every successful chunk on all platforms; verify with `ls-remote`; report mismatches |
| P12 | 3.5 HANDOFF | Restates Rule 6 (with cadence) alongside 11–13 |

## 2. Status after round 1 (verified against the tree at `abdc2a9`)

| Item | Round-1 status | Verified finding |
|---|---|---|
| P1 | "FIXED" | **Partly.** Remaining swallowers: `ci.yml` `killall ... \|\| true` and `auval -a \| grep ... \|\| true` (AU step). Worse, an unlisted class: the `sanitize` job pipes the test runner into `tee` in a default `bash -e` shell **without `pipefail`**, so a failing test run would still pass the step. |
| P2 | not assessed | Bench and Mull uploads now use `if-no-files-found: error`, but nothing checks the artifacts' *content* (non-empty, parses, expected fields). `plan/PLAN.md` Done-when lines carry no post-conditions. |
| P3 | "DONE (human-relaxed)" | T-050 is entirely non-gating; the relaxation (D-001n) covers macOS only. **T-051 latency is a numeric threshold that does not gate**: `grimamp_bench` records `latency.*.passes` but always exits 0. |
| P4 | OPEN | No baseline file, no check. |
| P5 | table added | Table exists in `plan/ENVIRONMENT.md`; no machine-readable form, nothing enforces it. |
| P6 | DONE | D-001..D-001p logged. **Error found:** `reviews/PROTOCOL_CONFORMANCE_v36.md` cites the 77.9 % mutation score to run 40771992301; that run had no working Mull step. The 77.9 % comes from run 40898109414. |
| P7 | FIXED (pins) | Pin table corrected. Missing: commit hashes for tag pins, a command ledger (`research/COMMAND_VERIFICATION.md`) tying every literal CI command/flag to evidence. |
| P8 | not assessed | See P1/P3: two violations. |
| P9 | DONE | Diff recorded in the conformance review; re-run after this round. |
| P10 | probe recorded in ENVIRONMENT.md | `.checkpoints/state.json` carries no environment class. The probe describes the generation sandbox only; this implementation/amendment session is a second environment and is classified separately. |
| P11 | ENV-BLOCKED (generation sandbox) | Not blocked in this session: pushes work. `ls-remote` verification is performed here. |
| P12 | DONE | HANDOFF restates Rule 6. |
| 3.7 closing steps | not assessed | `RETROSPECTIVE.md` was never written (S-RETRO); S-KNOW ran on the user's instruction (G-003 evidence recorded below). |

## 3. Amendment steps

Each step: **Action**, **Done when** (with Rule 14 post-conditions). Order matters; one commit and
one push per step (Rule 6 cadence), verified with `ls-remote`.

### A-1 Correct round 1's wrong evidence (P6)
- Action: in `reviews/PROTOCOL_CONFORMANCE_v36.md` change "77.9 % on run 40771992301" to run 40898109414, with a dated note.
- Done when: `grep -n 40771992301 reviews/PROTOCOL_CONFORMANCE_v36.md` shows no mutation-score claim.

### A-2 Remove remaining swallowed failures and the missing `pipefail` (P1, P8)
- Action: add `set -o pipefail` to both `sanitize` run blocks; replace `killall ... || true` with a conditional (`pgrep` guard) and drop the diagnostic `auval ... || true` line; add a `lint-ci` step to the job `build-test` that fails on `\|\| true`, `continue-on-error`, `set +e` outside an allow-list file (`tools/ci/swallow-allowlist.txt`, empty by default).
- Done when: `tools/ci/lint_ci.py` exits 0 on the tree and exits non-zero on a copy with `|| true` re-inserted (self-test run in CI); the lint step runs on every push.

### A-3 Gate T-051 latency (P3, P8)
- Action: new `tools/ci/check_bench.py` reads `bench-results.json`; fails if `latency.Eco.passes` or `latency.High.passes` is false.
- Done when (post-conditions): file exists, non-empty, parses as JSON, contains 7 voicings × 2 modes under `voicings` and both latency entries; script exits non-zero on a doctored copy (self-test).

### A-4 Per-runner-class T-050 targets, machine readable (P5, P3)
- Action: `tests/bench/bench-targets.json` — per runner label: `windows-latest` budget 15 % **enforced**; `macos-latest` budget 15 % status `relaxed` citing D-001n (reported, not enforced); Eco budget 15 % enforced on both. `check_bench.py` applies it and prints a verdict table into the job summary.
- Done when: CI job summary shows the table; a doctored result above budget fails on Windows and passes with a RELAXED line on macOS (self-test).
- **Human-gate boundary:** relaxing/restoring the macOS target stays a maintainer decision (Rule 14). This step encodes D-001n; it does not change it.

### A-5 Regression baseline bootstrap (P4)
- Action: `tests/bench/baseline-T050.json` — per runner class and voicing/mode CPU % taken from the CI artifacts of run 40898109414 (post gate-fix). `check_bench.py` fails when a Windows value exceeds `baseline × 1.20`; for `macos-latest` the same comparison is reported only (relaxed, D-001n). If the baseline has no entry for the runner class, the script writes `bench-baseline-candidate.json`, uploads it, prints `BASELINE RECORDED` and passes.
- Done when: baseline file parses and covers both runner classes; both branches (compare, bootstrap) exercised by self-tests; recipe and refresh procedure written into `tests/bench/README.md`.
- **Known risk, recorded not hidden:** run-to-run variance on hosted runners exceeds 20 % (K-20261029T030800Z; macOS 17–36 %, Windows 10.8–14.5 %), so the Windows gate may flake. Mitigation rule: a flake is investigated, never silenced; if it recurs the maintainer decides between a median-of-N bench, a wider tolerance (human gate) or a pinned runner.

### A-6 Artifact post-conditions in CI (P2)
- Action: after the bench step, run `check_bench.py` (A-3..A-5); in the mutation job the gate step first asserts `test -s mull.sqlite` and a minimum mutant count (≥ 100); sanitize job asserts `asan.log` contains `ALL TESTS PASSED`.
- Done when: each assertion present in `ci.yml`; deleting the artifact (self-test or by reading the step) fails the step.
- Plan side: `plan/POSTCONDITIONS_v36.md` lists, per plan step that produces an artifact, the post-condition and the CI/manual check that enforces it (amends every step's Done-when without rewriting `PLAN.md`).

### A-7 Command & pin verification ledger (P7)
- Action: `research/COMMAND_VERIFICATION.md` — every literal command/flag in `ci.yml` and `plan/ENVIRONMENT.md` with evidence (CI run id, `--help` capture, or doc locator for the pinned version); pin existence lookups run on 2026-10-31 with commit hashes (JUCE tag, pluginval tag, actions tags, mull package). Mark any unverifiable invocation and give its decision rule.
- Done when: the ledger has no row without evidence or an explicit "unverified + rule" entry; `tools/ci/lint_ci.py --ledger` fails if a `run:` command's first token is absent from the ledger (cheap completeness check).

### A-8 Environment class for this session (P10)
- Action: record in `.checkpoints/state.json` an `execution_environments` list: the generation sandbox (ephemeral, per round 1) and this session (Claude cloud/remote), probed now — `/tmp` files created in earlier turns still present, background commands survive across calls → class `stable`; `[ephemeral]` protections N/A here.
- Done when: `python3 -c` load of state.json shows both entries with probe evidence and date.

### A-9 Push-cadence verification (P11)
- Action: every step above is pushed on completion; after each push run `git ls-remote origin <branch>` and compare to `git rev-parse HEAD`; mismatches go to `EXECUTION_LOG.md`.
- Done when: `EXECUTION_LOG.md` has a verified-push row per step.

### A-10 S-RETRO (3.7.1) and G-003 record
- Action: write `RETROSPECTIVE.md` including question 1a (workflow/manifest diff vs `plan/ENVIRONMENT.md`, re-run after A-1..A-9) and the Rule 14 findings of this round; push to the Lessons store at `retrospectives/guitar-amp-vst/RETROSPECTIVE.md`. Record that lessons/knowledge were pushed on explicit user instruction on 2026-10-30 (stands in for G-003).
- Done when: file exists, has the 1a table, is present in the Lessons repo (fresh-clone check).

### A-11 Lessons and knowledge from this round (Rules 11, 12)
- Action: lesson on GitHub `run` steps having no `pipefail` by default (found by this audit) and on protocol-compliance review finding gaps in a previous "conformance" claim; knowledge item for the Actions default shell. Push to both stores (validate, index, fresh-clone check).

### A-12 Close-out
- Action: `DEVIATIONS.md` D-001q (this amendment), `EXECUTION_LOG.md` rows, update PR #1 description, watch CI to completion.
- Done when: CI for the final commit is green on all jobs, or any red job is explained in the PR.

## 4. Out of scope / human-gate items

- Relaxing or restoring the macOS T-050 budget (D-001n stands).
- Choosing median-of-N or a wider tolerance if the Windows gate proves flaky.
- The noise gate's 4096-sample RMS window vs the 5 ms design (D-001m).
- G-002 (release) is untouched.
