---
id: K-20261101T140200Z-test-executable-allocation-probe
statement: "Replacing the global operator new/delete in a JUCE test executable with versions that count calls made by the current thread (thread_local flag) gives a working zero-allocation check for processBlock; it needs a self-test that a deliberate allocation is counted, and it works alongside ASan/UBSan."
status: current
supersedes: []
tags: [subject:sanitizers-ci, tech:juce, tech:cpp, phase:verification]
applies_to_version: "clang/AppleClang/MSVC, JUCE 8.0.16"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/41300729228", "doi": "", "title": "no_audio_alloc test", "locator": "passed on macOS, Windows and under ASan with the probe self-check", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-11-01T14:02:00Z
---
Define operator new, new[], delete, delete[] and the sized deletes in one translation unit of the test binary; count only while `thread_local bool tracking` is set around the call under test, and count frees as well as allocations. Parameter changes belong outside the tracked region.
