# OPERATIONS — Release Runbook, Monitoring, Rollback (software.deploys = true)

## Release runbook (post G-002 sign-off)

1. Publish the drafted GitHub Release for tag `vX.Y.Z` (artifacts: VST3 Win, VST3 mac, AU mac, Standalone, `SHA256SUMS.txt`, release notes from `CHANGELOG.md`).
2. Post-release verification (within 24 h):
   - Download each artifact from the release page (not the CI cache) and verify against `SHA256SUMS.txt`.
   - Fresh machine/VM: install → Live rescan → load → play 30 s → uninstall (abbreviated T-060).
   - Confirm the release page renders and asset names match the README's install instructions.
3. Announce only where the human approved at G-002.

## Monitoring

- **Channel:** GitHub Issues only (no telemetry by design, D-010). Watch labels: `crash`, `audio-glitch`, `host:<name>`, `regression`.
- **Weekly triage** for the first month after release; severity ladder: crash-on-load > audio corruption > host-specific misbehavior > cosmetic.
- **Alert fatigue note:** there are no automated alerts; do not invent any. The "monitoring gap" risk is accepted and recorded as R-009 (Medium).

## Rollback

- **Broken release pulled:** mark the GitHub Release as pre-release/draft (or delete the release, keeping the tag) and post a pinned issue: "vX.Y.Z withdrawn: <reason>; use vX.Y.(Z-1)".
- **Rollback command:** `gh release delete vX.Y.Z --yes` (keeps the git tag; add `--cleanup-tag` only if the tag itself is wrong).
- **Hotfix path:** branch `hotfix-vX.Y.(Z+1)` from the bad tag, fix, run the full frozen suite + pluginval + abbreviated T-060, draft release, **G-002 sign-off again** (a hotfix is still an external action).
- **User data safety:** presets/IRs live outside the plugin bundle; uninstall/reinstall never touches them. State restore from old versions must keep working (T-009 tolerance rule in D-008) — a release that breaks old-session recall is a Critical defect and triggers immediate rollback.

## On-call / escalation

Single maintainer (bus factor R-010, Medium). Escalation = the repo owner; no SLA. Critical crash reports get a workaround note in the issue within 7 days on a best-effort basis.

## Data & privacy

Nothing to operate: no servers, no accounts, no analytics. The release artifacts are static binaries.
