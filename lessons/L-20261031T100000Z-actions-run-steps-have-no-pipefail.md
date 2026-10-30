---
id: L-20261031T100000Z-actions-run-steps-have-no-pipefail
title: A pipeline into tee in a default GitHub Actions run step cannot fail the step: the default shell has no pipefail
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [tech:github-actions, phase:verification, kind:verification-gap]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-31T10:00:00Z
---
## Trigger
A workflow `run:` block (no `shell:` key) contains `cmd | tee log` or any pipeline whose last command succeeds regardless of the first.

## What went wrong
The `sanitize` job ran the test executable as `./grimamp_tests ... 2>&1 | tee asan.log` in the default shell, which GitHub starts as `bash -e {0}` without `pipefail`. A failing sanitizer run would have exited non-zero in the first command and still passed the step. The job was green and was reported as evidence for two days before a protocol-conformance audit read the step against Rule 14. The mutation job had `set -o pipefail` because an earlier run had taught that; the later job copied the pattern without it.

## Correction
Add `set -o pipefail` as the first line of every `run:` block containing a pipe, or set `shell: bash` on the step (which implies `-eo pipefail`). Assert the verdict from the log as a post-condition (for example count the `ALL TESTS PASSED` lines) so the step fails even if the exit status is lost. Lint workflows for the pattern.

## Prevention rule
Every `run:` block with a pipe either starts with `set -o pipefail` or runs under `shell: bash`, and a CI lint enforces it.

## Detection check
`python3 tools/ci/lint_ci.py` (guitar-amp-vst) reports "pipeline into tee without pipefail"; its self-test inserts the pattern and must fail.

## Evidence
guitar-amp-vst run 40925278824 (green sanitize job, no pipefail); fixed in amendment round 2, step A-2.
