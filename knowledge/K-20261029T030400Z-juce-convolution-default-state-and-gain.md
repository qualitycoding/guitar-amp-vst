---
id: K-20261029T030400Z-juce-convolution-default-state-and-gain
statement: "A freshly prepared juce::dsp::Convolution already reports getCurrentIRSize() > 0 before any IR is loaded, and a loaded one-sample IR of 0.5 (Normalise::no, matching rate) produced an output gain of about 1.53 per unit input, i.e. JUCE applies an internal scale to loaded IRs."
status: current
supersedes: []
tags: [subject:juce-convolution, tech:juce, domain:audio, phase:verification]
applies_to_version: "8.0.16"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40896465359", "doi": "", "title": "cabir_selection test", "locator": "mic/mix expectations 0.5 vs actual 0.766938 for delta 0.5; first-readiness assertion failed", "accessed": "2026-10-30", "tier": 1}
confidence: single-source
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:04:00Z
---
Observed on the Linux/Mull build and on the Windows and macOS CI builds. Consequences: do not assert that an engine is empty before loading, and calibrate wet-path expectations against a measured reference render instead of the nominal IR value. Loading is asynchronous: poll `getCurrentIRSize()` (and process blocks) until the new IR is live.
