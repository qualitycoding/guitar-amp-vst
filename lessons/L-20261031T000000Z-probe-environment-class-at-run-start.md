---
id: L-20261031T000000Z-probe-environment-class-at-run-start
title: Environment-class probe skipped at run start cost days of rebuilds on an ephemeral sandbox
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:cmake, tech:git, phase:implementation, kind:environment]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-31T00:00:00Z
---
## Trigger
A run starts implementation (S-000 onward) in an agent sandbox without recording whether the environment is stable or ephemeral. Signals: /tmp contents vanish between turns, binaries copied from the durable mount refuse to run, background jobs disappear, the compiler reads stale bytes.

## What went wrong
The sandbox was ephemeral and showed it repeatedly: /tmp wiped twice mid-build, exec bits stripped on copies from the durable mount (ctest BAD_COMMAND), background nohup jobs reaped at tool-call boundaries, the mount serving stale object bytes, transient git index.lock failures. Each behaviour was discovered ad hoc mid-step and paid for in full rebuilds (13 to 20 minute LTO links) before any durable-surface discipline existed.

## Correction
The protocol was upgraded (v3.6 section 0.1 and 1.5) and this run's plan was amended (D-001o) with the probe result, the protections and tools/restage-env.sh.

## Prevention rule
Before the first build step of any run, classify the environment as stable or ephemeral with observed evidence and record it in the environment document; if ephemeral, put the restaging script and durable-surface layout in place before the first configure.

## Detection check
`grep -q ephemeral plan/ENVIRONMENT.md` succeeds before the first cmake invocation; a run with builds but no environment-class record fails review.

## Evidence
guitar-amp-vst plan/ENVIRONMENT.md section 0.1 and EXECUTION_LOG.md environment-reset entries.
