# RETROSPECTIVE — run-20261027T120000Z-guitar-amp-vst (protocol 3.7.1)

Written 2026-10-31 at the end of the implementation and amendment rounds. Every finding links to a
lesson (`L-`) or knowledge item (`K-`) pushed to `qualitycoding/Lessons` and `qualitycoding/knowledge`.

## 1. What went wrong, and what was corrected

| Finding | Correction | Record |
|---|---|---|
| Plan pinned a pluginval version and an action tag that do not exist; default binary of the action aborts on macOS | pin action and binary to `v2.0.0-beta3`; D-001i | L-20261028T200000Z-pinned-tool-version-does-not-exist, K-20261029T030000Z-pluginval-releases-and-action |
| CRLF checkout broke the byte-hash manifest on Windows | `.gitattributes` `tests/** text eol=lf` | L-20261028T210000Z-crlf-breaks-hash-manifests-on-windows, K-20261029T030600Z-windows-checkout-crlf-hash-manifests |
| Empty VST3 program name passed every local check, failed the Steinberg validator | `getProgramName` returns "Default" | L-20261028T215000Z-vst3-validator-not-in-local-suite |
| Noise gate never attenuated (`decibelsToGain(db, 1e-6f)`) | argument removed; D-001m | L-20261028T220000Z-decibelstogain-minusinfinity-misuse, K-20261029T030300Z-juce-decibelstogain-second-argument |
| Mutation job green while Mull never ran, then green at 37.7 % | gate step reads the SQLite report | L-20261029T000000Z-green-ci-gate-never-ran, K-20261029T030100Z/-030200Z |
| Mull per-mutant cost made a 12.5 min suite infeasible | fast test subset; D-001l | L-20261029T010000Z-mull-reruns-whole-suite-per-mutant |
| Sanitizer job could not fail (no pipefail); latency verdict never gated; round-1 conformance review marked partial items DONE | amendment round 2 | L-20261031T100000Z-actions-run-steps-have-no-pipefail, L-20261031T100100Z-verify-conformance-claims-against-the-tree, K-20261031T100200Z-github-actions-default-shell-pipefail |

Corrections made by reading survivors from the lowest-scoring file first: L-20261029T020000Z-start-mutation-review-at-lowest-file.

## 1a. Completeness diff: workflows and manifests vs `plan/ENVIRONMENT.md`

Diffed at the commit that closes round 2.

| Difference from the plan | Logged in `DEVIATIONS.md` |
|---|---|
| JUCE 8.0.16-security instead of 8.0.15 | D-001 |
| pluginval v2.0.0-beta3 (action ref, binary, inputs, `--timeout-ms 180000`) | D-001i, D-001j |
| lld-19, JUCE Linux dev packages, AU build + registration, `.gitattributes` | D-001j |
| `mull.yml`, Mull fast subset, score gate | D-001l, D-001q |
| `sanitize` job (ASan + UBSan, fuzz corpus) | **was unlogged; logged now in D-001q** (this check found it) |
| `tests/MutationTests.cpp` and its wiring in `tests/CMakeLists.txt` | D-001m covers the gate fix; the test file itself is logged in `reviews/MUTATION.md`, **added to D-001q references** |
| `lint-ci`, `check_bench.py`, bench targets and baseline, post-conditions, ledger | D-001q |
| `|| true` / missing pipefail removed | D-001p, D-001q |
| `release.yml` | untouched in this run; unverified commands listed in the ledger with a rule |

`EXECUTION_LOG.md` has one row per recorded step attempt. Known gap: the early sandbox rows
(S-001..S-013) were written before CI existed and some record "uncommitted" instead of commit ids.

## 2. What was overlooked

- **`S-REVIEW` and `S-ADVERSARIAL` (protocol 3.8) were never executed.** `reviews/CODE_REVIEW.md` and
  `reviews/ADVERSARIAL_REVIEW.md` do not exist. They need fresh-context reviewers who did not author
  the code. This is an open Critical-process gap, not a closed item; they are the next work before any
  G-002 request.
- No checks existed for the executables' exit-status paths: the bench harness always returned 0, so a
  gating threshold in the spec (T-051) could not fail CI (fixed round 2).
- The Windows T-050 gate has a thin margin and hosted runners vary more than the regression tolerance
  (K-20261029T030800Z-github-hosted-runner-benchmark-variance). A flake is to be investigated, not silenced.
- The noise gate's "5 ms" RMS window is a 4096-sample box (D-001m); loudness calibration was not
  re-run after the gate fix.

## 3. R0 lessons not applied or recurring

The R0 store read at planning time held no applicable lesson on CI honesty. Recurrence inside this run:
the "check that cannot fail" pattern occurred three times (swallowed Mull error, inert threshold flag,
missing pipefail) and each time was only caught by reading the produced number. The Rule 14 lint now
covers the third; the first two are covered by the Mull gate and the post-conditions.

## 4. Facts not yet knowledge items

None outstanding: the findings above each link to a pushed K- item. The unreleased packaging path
(`release.yml`) has facts nobody has learned yet.

## 5. G-003 record

Lessons and knowledge from the planning run and from implementation were pushed to the stores on the
maintainer's explicit instruction (2026-10-29/30), which stands in for the G-003 sign-off. Round 2's
items are pushed under the same standing instruction.
