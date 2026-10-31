---
id: L-20261029T010000Z-mull-reruns-whole-suite-per-mutant
title: Mull re-runs the whole test executable once per mutant, so a 12-minute suite makes mutation testing infeasible without a subset
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:mull, tech:cpp, phase:implementation, kind:process]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T01:00:00Z
---
## Trigger
Adding Mull to a project whose test executable takes minutes to run, or whose baseline run times out at Mull's default.

## What went wrong
The first real Mull run failed its warm-up with `status: Timedout` at the default 3 s. After raising the timeout, the full suite took 12.5 minutes under `-O0` instrumentation; with hundreds of mutants this cannot fit in a CI job. A dry run also executes the baseline, wasting another 12 minutes.

## Correction
Pass test names as runner arguments so only the fast tests run per mutant (documented as a deviation), set `--timeout` and `--minimum-timeout` explicitly, run with `--workers` equal to the core count, and skip the dry-run step. Tests that depend on a ctest fixture (for example a generated fuzz corpus) must be excluded or given their environment.

## Prevention rule
Time the unit executable before wiring up Mull; budget mutants x suite time / workers against the job limit, and choose the test subset up front.

## Detection check
The Mull log's `Warm up run ... Finished in` line is below a minute or two.

## Evidence
guitar-amp-vst runs 40841867748 (3 s warm-up timeout), 40843193060 (12m29s warm-up, 2 fuzz failures), 40847181602 (subset, 66 min for 154 mutants).
