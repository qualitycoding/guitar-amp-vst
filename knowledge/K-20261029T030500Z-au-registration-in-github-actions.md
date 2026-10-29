---
id: K-20261029T030500Z-au-registration-in-github-actions
statement: "To validate an AU component with pluginval on a GitHub macOS runner, build the AU target explicitly, copy the .component to ~/Library/Audio/Plug-Ins/Components, ad-hoc sign it (codesign --force --deep -s -), restart AudioComponentRegistrar and pass the installed path; the unregistered build-tree path fails with 'No types found'."
status: current
supersedes: []
tags: [subject:juce-au-ci, tech:juce, tech:github-actions, domain:audio, phase:verification]
applies_to_version: "macos-latest (Apple Silicon), pluginval v2.0.0-beta3"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40771992301", "doi": "", "title": "green macOS job", "locator": "step 'Register AU (macOS only)' then pluginval AU step", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:05:00Z
---
Before the fix, pluginval reported `Scan for plugins located in .../AU/GrimAmp.component: Test 1 failed: No types found` because the CI built only the VST3 target and the component was not registered. The registration step plus an explicit `--target GrimAmp_AU` build made the AU test pass.
