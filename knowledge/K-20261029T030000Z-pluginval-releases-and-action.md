---
id: K-20261029T030000Z-pluginval-releases-and-action
statement: "pluginval has no stable 2.0.0 release: the published 2.x releases are v2.0.0-beta1, -beta2 and -beta3 (beta3 newest as of 2026-10-29), alongside v1.0.x; the GitHub Action lives at Tracktion/pluginval/validate and has no v2 tag."
status: current
supersedes: []
tags: [subject:pluginval, tech:pluginval, tech:github-actions, domain:audio, phase:implementation]
applies_to_version: "v2.0.0-beta3"
source: {"url": "https://github.com/Tracktion/pluginval/releases", "doi": "", "title": "pluginval releases", "locator": "gh api repos/Tracktion/pluginval/releases; contents/validate/action.yml at v2.0.0-beta3", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:00:00Z
---
Action inputs at v2.0.0-beta3: `plugins` (one path/glob/AU id per line), `strictness-level` (default 5), `args` (extra options such as `--timeout-ms 180000`), `report-dir`, `pluginval-version` (default `v2.0.0-beta1`), `pluginval-path`. The default beta1 binary aborts on macOS (missing `librtcheck.dylib`); set `pluginval-version: v2.0.0-beta3`. Release assets: `pluginval_Linux.zip`, `pluginval_macOS.zip`, `pluginval_Windows.zip`. The older `pluginpath` / `strictness` input names are not accepted.
