---
id: K-20261029T030300Z-juce-decibelstogain-second-argument
statement: "juce::Decibels::decibelsToGain (db, minusInfinityDb = -100) treats its second argument as a dB value: any db not greater than minusInfinityDb returns 0, so passing a positive linear-looking value such as 1.0e-6f makes every realistic dB input return 0."
status: current
supersedes: []
tags: [subject:juce, tech:juce, domain:audio, phase:implementation]
applies_to_version: "8.0.16"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40867662636", "doi": "", "title": "gate_reference test", "locator": "failed on run 40866289187 with the call, passed after removing the second argument", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:03:00Z
---
Verified experimentally: a noise gate using `decibelsToGain (thrCloseDb, 1.0e-6f)` never attenuated (threshold gain 0), and the gate reference test passed once the argument was removed. See the lesson on this misuse.
