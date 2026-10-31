---
id: K-20261031T100200Z-github-actions-default-shell-pipefail
statement: "GitHub Actions run steps without a shell key use 'bash -e {0}' on Linux and macOS (no pipefail), whereas 'shell: bash' expands to 'bash --noprofile --norc -eo pipefail {0}', so only the explicit form makes a failing first command in a pipeline fail the step."
status: current
supersedes: []
tags: [subject:github-actions-shell, tech:github-actions, phase:verification]
applies_to_version: "GitHub-hosted runners, 2026-10"
source: {"url": "https://docs.github.com/actions/using-workflows/workflow-syntax-for-github-actions", "doi": "", "title": "Workflow syntax: jobs.<job_id>.steps[*].shell", "locator": "default shell and shell: bash expansion; job log header 'shell: /usr/bin/bash -e {0}' in run 40843193060", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-31T10:02:00Z
---
The default form was observed in the job log header of the Mull step. Windows runners default to PowerShell, so a `run:` block there needs `shell: bash` to use the same semantics. A pipeline into `tee` is the common trap: the step exit status is `tee`'s.
