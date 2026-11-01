---
id: K-20261029T030800Z-github-hosted-runner-benchmark-variance
statement: "CPU-percentage benchmarks on GitHub-hosted runners vary a lot run to run: the same code measured 10.8-11.7% on windows-latest in one run and 13.3-14.5% in another (AMD EPYC 9V74 vs 7763), and 18.0-35.6% versus 17.0-18.3% on the macos-latest virtual Apple M1 with 3 cpus."
status: current
supersedes: []
tags: [subject:github-hosted-runner-performance, tech:github-actions, phase:verification]
applies_to_version: "windows-latest and macos-latest, 2026-10"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40898109414", "doi": "", "title": "bench-results artifacts", "locator": "compare with run 40771992301", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:08:00Z
---
The machine string in the bench output differed between runs on the same runner label. A fixed CPU budget or a regression threshold of 20% against a previous run will therefore be flaky unless runs are repeated and the median or minimum is used, or a self-hosted runner is pinned.
