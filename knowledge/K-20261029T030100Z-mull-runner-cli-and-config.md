---
id: K-20261029T030100Z-mull-runner-cli-and-config
statement: "mull-runner-19 (Mull from the mull-stable Cloudsmith repository) takes `<EXECUTABLE> [RUNNER_ARGS]...` with options such as --workers, --timeout, --minimum-timeout, --reporters, --report-name, --mutation-score-threshold and --allow-surviving; it has no --include-path or --exclude-path option, which belong in mull.yml (includePaths and excludePaths regexes, mutators list)."
status: current
supersedes: []
tags: [subject:mull, tech:mull, tech:cpp, phase:implementation]
applies_to_version: "mull-19 (mull-stable, 2026-10)"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40841867748", "doi": "", "title": "mull-runner --help in CI", "locator": "job log of the Run Mull step", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:01:00Z
---
Observed: passing `--include-path` gives `error: unexpected argument '--include-path' found`; a `mull.yml` in the repository root is found at compile and run time (`[info] Using config .../mull.yml`; without it `[warning] Mull cannot find config`). Warm-up uses `--timeout` (default 3 s) and fails with `status: Timedout`. `--mutation-score-threshold` defaults to 100 and, combined with `--allow-surviving`, did not fail a 37.7% run (see lesson on green CI gates). The compile step needs `-fpass-plugin=/usr/lib/mull-ir-frontend-19`, clang-19 and, with `-fuse-ld=lld-19`, the `lld-19` package.
