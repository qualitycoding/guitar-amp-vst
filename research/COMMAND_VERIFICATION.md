# COMMAND & PIN VERIFICATION LEDGER (protocol v3.6, 1.3.6)

Created in amendment round 2 (A-7), 2026-10-31. Every literal command or flag in
`.github/workflows/ci.yml` and `plan/ENVIRONMENT.md` appears with the evidence that it works
against the **pinned** version. "CI run" evidence means the step passed in that run. Lookups
were made with `gh api` on 2026-10-31.

## 1. Pin existence

| Pin | Lookup | Result |
|---|---|---|
| JUCE `8.0.16-security` | `gh api repos/juce-framework/JUCE/git/ref/tags/8.0.16-security` | annotated tag object `a74b024c188339fdfe07b73088e6f6961ff66709`, peels to commit `ea77444750de45e036e0279bb0291144bf8e3688` (matches the CI submodule-pin check) |
| pluginval `v2.0.0-beta3` | `gh api repos/Tracktion/pluginval/git/ref/tags/v2.0.0-beta3` | tag object `442d7de9ec40675355e840d7bb3492e176d3f800`; action resolved to commit `37d6a37bc752606c524f7f3febffc73e0945a740` in CI run 40763261991 |
| pluginval `2.0.0` (original plan pin) | same API | **does not exist** (only `v2.0.0-beta1..3`, `v1.0.x`); replaced, D-001i |
| `actions/checkout@v4` | `git/ref/tags/v4` | commit `11d5960a326750d5838078e36cf38b85af677262` |
| `actions/upload-artifact@v4` | `git/ref/tags/v4` | commit `ea165f8d65b6e75b540449e92b4886f43607fa02` |
| Mull `mull-19`, `clang-19`, `lld-19` | installed by `setup.deb.sh` + `apt-get install` in the `mutation` job | install succeeded in CI run 40898109414 (package versions are whatever the `mull-stable` repo serves; the `19` suffixes must agree, K-20261029T030100Z) |
| `ubuntu-latest`, `windows-latest`, `macos-latest` | GitHub-hosted labels | all three exercised in CI run 40928176416 |

## 2. Commands and flags

| Command / flag | Evidence |
|---|---|
| `cmake -B build -DCMAKE_BUILD_TYPE=...`, `cmake --build build --config ... --target ...` | CI run 40898109414 (all three OSes) |
| `--target GrimAmp_VST3`, `GrimAmp_AU`, `grimamp_tests`, `grimamp_bench` | targets exist in `CMakeLists.txt` / `tests/CMakeLists.txt`; built in CI run 40898109414 |
| `sha256sum -c tests/FROZEN_MANIFEST.sha256` | CI run 40763261991 onward (LF pinned by `.gitattributes`, L-20261028T210000Z) |
| `ctest --test-dir build --output-on-failure -C <cfg>`, `-R bench` | CI run 40898109414 |
| `Tracktion/pluginval/validate@v2.0.0-beta3` inputs `plugins`, `strictness-level`, `args`, `pluginval-version` | `validate/action.yml` at the tag (K-20261029T030000Z); CI run 40769139053 |
| pluginval CLI `validate`, `--strictness-level`, `--timeout-ms` (default 30000), `--skip-gui-tests` | `docs/Command line options.md` at `v2.0.0-beta3` (read 2026-10-31) |
| macOS AU: `codesign --force --deep -s -`, `pgrep -x`/`killall -9 AudioComponentRegistrar`, copy to `~/Library/Audio/Plug-Ins/Components` | CI run 40771992301 (K-20261029T030500Z); the `pgrep` guard form is exercised by the next CI run (see section 4) |
| `dumpbin //imports "$BIN"` (Windows, Git Bash) | CI run 40898109414; failure now fails the step (D-001p) |
| `mull-runner-19 --workers --timeout --minimum-timeout --reporters SQLite IDE --report-name --allow-surviving` | `mull-runner-19 --help` captured in CI run 40843193060 (K-20261029T030100Z); used in run 40898109414 |
| `mull.yml` (`mutators`, `includePaths`, `excludePaths`) | "Using config .../mull.yml" in the run 40843193060 log; part of the Mull pin |
| `clang -fsanitize=address,undefined -fno-sanitize-recover=undefined`, `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` | CI run 40925278824 (K-20261029T030700Z) |
| `python3 tests/fuzz_preset.py --seed 42 --count 1000 --out-dir ... --seed-preset ...` | CI run 40925278824; same call as `tests/CMakeLists.txt` |
| `python3 tools/ci/lint_ci.py [--selftest]`, `python3 tools/ci/check_bench.py [--selftest]` | self-tests run locally on 2026-10-31 (9 cases, all ok); CI confirmation in section 4 |
| `gh run ...`, `gh api ...` (operator-side, not in plan) | n/a |

## 3. Unverified invocations (explicit spike + decision rule, 1.3.6 item 4)

`.github/workflows/release.yml` (`hdiutil`, `mv`, `powershell` packaging commands) has never run:
the release path is behind gate G-002 and is untouched by this run. Rule: the first action of
step S-014/S-017 is a dry-run of `release.yml` on a throwaway tag in a fork; if any command fails
there, fix it before the G-002 request, and log the substitution in `DEVIATIONS.md`.

Otherwise none remain. The one command in `plan/ENVIRONMENT.md` that diverged from CI reality was the macOS
AU `pluginval validate ... AU/GrimAmp.component` line, which fails with "No types found" unless
the component is first copied, signed and registered; `plan/ENVIRONMENT.md` now says so.

## 4. Pending confirmation

Filled in by the close-out step (A-12) with the CI run id that exercised the new steps:
`check_bench.py` on Windows (`python` interpreter) and macOS, the `pgrep` AU guard, the
sanitizer post-conditions, and the `lint-ci` job.
