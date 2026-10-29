---
id: L-20261028T215000Z-vst3-validator-not-in-local-suite
title: The Steinberg VST3 validator inside pluginval rejected an empty program name that the local suite could not see
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:vst3, tech:juce, phase:verification, kind:verification-gap]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-28T21:50:00Z
---
## Trigger
A JUCE plugin whose `getNumPrograms()` is 1 or more and whose `getProgramName()` returns an empty string.

## What went wrong
The local frozen suite and JUCE's own checks passed. pluginval's embedded VST3 validator failed with "Programlist 000->Program 000: has no name" because VST3 requires a non-empty program name. On Windows the same validator step hung until the 30 s timeout.

## Correction
Return a real name (`"Default"`) and give pluginval a longer per-test timeout (`--timeout-ms 180000`).

## Prevention rule
Run pluginval (strictness 5 or higher) in CI from the first build that produces a plugin binary; do not rely on unit tests for host-contract rules.

## Detection check
pluginval's "vst3 validator" test passes on every OS in CI.

## Evidence
guitar-amp-vst D-001k; runs 40766274769 (macOS fail), 40769139053 (pass on both).
