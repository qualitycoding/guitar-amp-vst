---
id: K-20261029T030900Z-gh-run-logs-and-escape-sequences
statement: "gh api repos/<o>/<r>/actions/jobs/<id>/logs refuses output containing terminal escape sequences unless --allow-escape-sequences is passed, and returns a BlobNotFound 404 until the job has completed, so a running job's log cannot be read; gh run view <id> --log-failed is also unavailable while the run is in progress."
status: current
supersedes: []
tags: [subject:github-actions-logs, tech:github-actions, tech:github, phase:verification]
applies_to_version: "gh 2.x, 2026-10"
source: {"url": "https://cli.github.com/manual/gh_api", "doi": "", "title": "gh api manual", "locator": "observed in this session", "accessed": "2026-10-30", "tier": 1}
confidence: single-source
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:09:00Z
---
Strip colour codes with `sed 's/\x1b\[[0-9;]*m//g'` and remove carriage returns (`tr -d '\r'`) before grepping. Job step statuses (`gh run view --json jobs`) are readable during the run and show which step is still executing.
