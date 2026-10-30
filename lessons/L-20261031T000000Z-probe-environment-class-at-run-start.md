# L-20261031T000000Z — environment-class probe skipped at run start cost days of rebuilds

- **trigger:** correction / missed protocol step
- **mistake:** the run started implementation (S-000 onward) without recording the
  §0.1 environment-class probe. The sandbox was `ephemeral` and demonstrated it
  repeatedly — /tmp wiped between turns (twice mid-build), exec bits stripped on
  copies from the durable mount (ctest BAD_COMMAND), background `nohup` jobs reaped
  at tool-call boundaries, the durable mount serving stale object bytes to the
  compiler, transient git index.lock failures — but each behavior was discovered
  ad hoc, mid-step, and paid for in full rebuilds (13–20 min LTO links) before the
  durable-surface/restaging discipline existed.
- **correction:** protocol upgraded the hard-won rules into v3.6 §0.1 + §1.5; this
  run's plan was then amended (D-001o) with the probe result, the protections
  section, and tools/restage-env.sh.
- **prevention rule (imperative, checkable):** at the start of ANY run, before the
  first build step, classify the environment (stable vs ephemeral) with observed
  evidence and record it in the plan's environment document; if ephemeral, put the
  restaging script and durable-surface layout in place BEFORE the first configure.
- **detection check:** `grep -q "ephemeral" plan/ENVIRONMENT.md` (or the run's
  environment doc) must succeed before the first `cmake` invocation; a run with
  builds but no environment-class record fails review.
- **tags:** environment, ephemeral, sandbox, planning-protocol, build-hygiene
