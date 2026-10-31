---
id: L-20261031T100100Z-verify-conformance-claims-against-the-tree
title: A previous conformance review marked items DONE that the tree did not fully satisfy; a second pass found swallowed failures, an ungated latency threshold and a wrong run id
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:github-actions, phase:verification, kind:process]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-31T10:01:00Z
---
## Trigger
A review document or hand-off says a protocol or checklist item is DONE or FIXED, and a later pass is asked to re-check against the same protocol.

## What went wrong
Round 1 of the v3.6 conformance review listed the no-swallowed-failures item as fixed and the latency/budget items as done. Reading the workflows and the bench harness showed `|| true` still present in the AU step, a `| tee` without pipefail in the sanitizer job, `grimamp_bench` always returning 0 so the T-051 latency verdict never failed the build, and the 77.9 % mutation score attributed to a run whose Mull step had not executed the mutants.

## Correction
Re-derive each claim from the artifact it is about (the YAML line, the exit path of the harness, the run's own logs), not from the earlier prose; correct the earlier document with a dated note instead of silently editing it.

## Prevention rule
Treat a "DONE" in a prior review as a hypothesis: for each item run the grep, read the exit path, or open the run, and record the evidence next to the verdict.

## Detection check
Each verdict row in a conformance review cites a file:line, a command output or a run id that a reader can open.

## Evidence
guitar-amp-vst `plan/AMENDMENT_PLAN_v36_r2.md` section 2 (verified findings) and step A-1.
