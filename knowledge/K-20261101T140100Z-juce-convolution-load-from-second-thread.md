---
id: K-20261101T140100Z-juce-convolution-load-from-second-thread
statement: "Calling juce::dsp::Convolution::loadImpulseResponse from a worker thread while the audio thread processes, with prepare() and the audio thread also touching the object, made random parameter automation throw std::bad_function_call in a JUCE 8.0.16 plugin (reproduced under ASan/UBSan at block size 1024 and by pluginval on macOS and Windows); keeping the calls on the audio thread did not."
status: current
supersedes: []
tags: [subject:juce-convolution, tech:juce, domain:audio, phase:verification]
applies_to_version: "8.0.16"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/41301668413", "doi": "", "title": "sanitize job log", "locator": "automation_stress: exception at rate 44100 block 1024: bad_function_call", "accessed": "2026-10-30", "tier": 1}
confidence: single-source
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-11-01T14:01:00Z
---
Cause not confirmed from JUCE source (not checked out); the working hypothesis is that the convolution message queue assumes a single producing thread. The audio-thread call decodes the WAV (factory cab) or copies the IR on the calling thread, so it is not allocation-free either; a design that keeps all Convolution API calls on one non-audio thread has not been tried.
