# POSTCONDITIONS — Rule 14 artifact post-conditions per plan step

Amends the `Done when` of each step in `plan/PLAN.md` (protocol v3.6, amendment round 2, A-6)
without rewriting the plan. A step that produces an artifact is not done until its row holds.
"Enforced by" names the CI step or manual check; a row with only a manual check is a known gap.

| Step | Artifact | Post-condition (exists + sanity) | Enforced by |
|---|---|---|---|
| S-001..S-008 build | `GrimAmp.vst3`, test and bench binaries | exist after build; ctest finds the tests | `build-test` Build + ctest steps |
| S-011 frozen suite | `FROZEN_MANIFEST.sha256` verification | every listed file hashes OK on Linux, macOS and Windows | `Freeze manifest check` (all runners; `.gitattributes` pins LF) |
| S-012/S-013 pluginval | pluginval verdict | VST3 (both OSes) and AU (macOS): 0 failed | pluginval steps (exit code) |
| S-013 bench | `bench-results.json`, `bench-baseline-candidate.json` | exist, non-empty, parse, 7 voicings x 2 modes + both latency entries | `Check bench gates` + `Upload bench results` (`if-no-files-found: error`) |
| S-013 T-051 | latency verdict | `latency.Eco.passes` and `latency.High.passes` true | `tools/ci/check_bench.py` |
| S-013 T-050 | CPU % | Windows: Frost/High <= 15 % and no >20 % regression vs baseline; macOS: reported (D-001n) | `tools/ci/check_bench.py`, `tests/bench/bench-targets.json` |
| S-MUTATE T-070 | `mull.sqlite`, `mull.log` | non-empty; >= 100 mutants; score >= 75 % | `Mutation score gate`, `Upload Mull report` |
| S-ASAN (added) | `asan.log` | non-empty; exactly two `ALL TESTS PASSED` | `Assert sanitizer runs completed`, `Upload sanitizer log` |
| T-040 import table | `imports.txt` | VST3 binary found; `dumpbin` succeeded; no forbidden imports | `T-040 import-table check` |
| CI hygiene | workflows | no `\|\| true`, `continue-on-error`, `set +e`; no pipe into `tee` without pipefail | `lint-ci` job (`tools/ci/lint_ci.py`) |
| S-014 release | installers, checksums | not produced in this run (G-002 pending) | n/a |
| S-RETRO | `RETROSPECTIVE.md` | exists; contains the 1a completeness table | manual (A-10) |
| S-KNOW | store commits | every pushed ID appears in the store's `INDEX.md` (fresh-clone check) | manual (A-10, A-11) |
