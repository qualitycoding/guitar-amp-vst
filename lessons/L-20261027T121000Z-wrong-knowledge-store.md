---
id: L-20261027T121000Z-wrong-knowledge-store
title: Knowledge store location taken from protocol default instead of verified with the human
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [phase:intake, kind:wrong-assumption, applies:all]
recorded_by: planner
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-27T12:10:00Z
---
## Trigger
Intake asks for a knowledge store (Rule 13) and the human has not named one explicitly. Temptation: adopt the protocol's default proposal (`agent-knowledge`) without flagging it as a guess needing confirmation.

## What went wrong
The planner recorded `qualitycoding/agent-knowledge` as the knowledge store (A-008) by adopting the protocol's default proposal. The human corrected it: the actual store is `qualitycoding/knowledge`. The wrong location had already propagated into ASSUMPTIONS.md, PRIOR_KNOWLEDGE.md, GATES.md, HANDOFF.md, and the research round log.

## Correction
Human correction received; A-008 amended to `qualitycoding/knowledge`, all five artifacts updated, DR-9 added so S-000 re-verifies store read access with implementer credentials, R-013 recorded for the residual ambiguity (404 = private or absent).

## Prevention rule
When the intake batch includes a knowledge store, always present the proposed location explicitly as its own numbered intake item and probe its existence (`git ls-remote` / repo API) before recording it; never silently adopt the protocol default.

## Detection check
`plan/ASSUMPTIONS.md` A-008 shows a probed location (access attempt result recorded) and matches what the human stated; `.checkpoints/state.json` `knowledge_store.location` equals the same value.

## Evidence
Human message: "The knowledge store you were given was incorrect. It should be qualitycoding/knowledge." Amendment commit on the generation branch. No secrets.
