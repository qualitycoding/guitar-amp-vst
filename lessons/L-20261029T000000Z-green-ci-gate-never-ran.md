---
id: L-20261029T000000Z-green-ci-gate-never-ran
title: A green CI job did not mean its gate ran: error-swallowing and an inert threshold flag hid a 37.7% mutation score
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [tech:github-actions, tech:mull, phase:verification, kind:verification-gap]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T00:00:00Z
---
## Trigger
A CI step ends in `|| true`, uses a flag like `--allow-surviving`, or is labelled informational, and the job is reported green.

## What went wrong
The mutation job was green for several runs while (a) `mull-runner` failed on an unsupported option and `|| true` swallowed the error, so no report existed, then (b) after the runner worked, `--allow-surviving` made `--mutation-score-threshold 75` inert, so a 37.7% score passed. The macOS CPU benchmark likewise exceeded its 15% budget (up to 35.6%) while the job was green because the step was informational. Each time the green tick was read as evidence.

## Correction
Remove error-swallowing from gate steps, enforce the threshold in an explicit step that reads the produced report and exits non-zero, and read the actual number from the artifact before declaring the gate satisfied.

## Prevention rule
For every CI gate, confirm once that it can fail: feed it a failing input or read the produced number and compare it to the threshold yourself. Treat `|| true`, `continue-on-error`, `--allow-*` flags and "informational" steps as not gating.

## Detection check
`grep -n "|| true\|continue-on-error\|allow-surviving" .github/workflows/*.yml`; each hit needs a stated reason and a separate enforcing step.

## Evidence
guitar-amp-vst runs 40841867748 (runner usage error swallowed), 40847181602 (green at 37.7%), 40898109414 (gate step enforcing, 77.9%).
