---
id: K-20261031T000000Z-ephemeral-sandbox-invariants
statement: "On an ephemeral agent sandbox (observed: Kimi on cloud) /tmp is wiped between turns, the durable mount strips exec bits and can serve stale bytes, nohup jobs die with the tool call (setsid survives), CMake ignores environment-variable dependency overrides at configure time, git index.lock writes fail transiently, and outbound traffic to github.com can be blocked."
status: current
supersedes: []
tags: [subject:ephemeral-sandbox-build-hygiene, tech:cmake, tech:git, phase:implementation]
applies_to_version: "Kimi cloud sandbox, GCC 12.2, CMake 3.25, ninja (2026-10)"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/blob/impl-run-20261027T120000Z-guitar-amp-vst/plan/ENVIRONMENT.md", "doi": "", "title": "guitar-amp-vst plan/ENVIRONMENT.md sections 0.1 and 1.5", "locator": "EXECUTION_LOG.md environment-reset entries", "accessed": "2026-10-31", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-31T00:00:00Z
---
First-hand observation over several turns, 2026-10-27 to 2026-10-31. Facts:

1. `/tmp` is wiped between tool calls/turns; anything long-lived must live on the
   durable mount (`/mnt/agents`) and be restaged per turn.
2. The durable mount strips exec bits on copies — `chmod +x` binaries after every
   copy/relink or ctest fails with BAD_COMMAND.
3. `nohup ... &` jobs die with the tool call; `setsid ... < /dev/null > log 2>&1 &`
   survives. Poll the log for completion markers instead of sleeping.
4. The mount can serve STALE bytes to compilers and to git (one link mixed old and
   new object code), and `touch` does not update mtimes — ninja then never sees
   edits. Never build on the mount; build from a fresh /tmp copy; force rebuilds by
   deleting `.o`/`.a`; verify source state by content (`grep`), not timestamp.
5. CMake IGNORES environment-variable dependency overrides at configure time
   (`CPLUS_INCLUDE_PATH`, `LIBRARY_PATH`, `PKG_CONFIG_PATH` do not enter the cache);
   export them for every build command or pass them as `-D` cache arguments.
6. git `index.lock` writes fail transiently on the mount — bounded retry loop
   (succeeded on attempt 3 here).
7. Outbound git/network traffic to github.com is blocked (curl exit 124) while
   read-only web/raw-file fetches work — push cadence must use a bundle handoff
   with human-side `ls-remote` verification (protocol Global Rule 6 fallback).
