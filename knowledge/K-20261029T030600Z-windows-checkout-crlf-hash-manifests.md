---
id: K-20261029T030600Z-windows-checkout-crlf-hash-manifests
statement: "On GitHub's windows-latest runner actions/checkout produces CRLF line endings for text files by default, which breaks sha256sum -c on a manifest (file names read with a trailing carriage return); a .gitattributes line such as 'tests/** text eol=lf' fixes it."
status: current
supersedes: []
tags: [subject:git-line-endings, tech:git, tech:github-actions, phase:verification]
applies_to_version: "windows-latest, git 2.56"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40761557039", "doi": "", "title": "failing Windows job", "locator": "sha256sum: 'tests/TEST_SPEC.md'$'\r': No such file or directory", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:06:00Z
---
Passed on run 40763261991 after adding `.gitattributes`. Applies to any byte-hash manifest, not only this repository.
