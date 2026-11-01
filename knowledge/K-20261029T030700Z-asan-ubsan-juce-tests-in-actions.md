---
id: K-20261029T030700Z-asan-ubsan-juce-tests-in-actions
statement: "A JUCE console test executable builds and runs cleanly on ubuntu-latest with clang, -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined, linking with the same flags, and ASAN_OPTIONS=detect_leaks=0:halt_on_error=1; the build plus 6143 checks (including a 1000-file fuzz corpus) finished in under 10 minutes."
status: current
supersedes: []
tags: [subject:sanitizers-ci, tech:github-actions, tech:juce, tech:cpp, phase:verification]
applies_to_version: "ubuntu-latest, clang (runner default)"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40925278824", "doi": "", "title": "sanitize job", "locator": "6143 ok, 0 failed; steps all success", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:07:00Z
---
Needs the JUCE Linux dev packages (libasound2-dev, libx11-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev, libxext-dev, libfreetype-dev, libfontconfig1-dev, libgl1-mesa-dev). Leak detection is disabled because JUCE has its own leak detector. Omit tests that render long audio (they are slow under sanitizers).
