---
id: K-20261027T120000Z-live-vst3-support
statement: Ableton Live hosts 64-bit VST3 plugins on Windows and macOS (VST3 supported as of Live 10.1); AU (v2/v3) is macOS-only; AAX is unsupported. VST3 install paths Live scans: C:\Program Files\Common Files\VST3\ (Win) and /Library/Audio/Plug-Ins/VST3/ (macOS). On Apple Silicon, the Universal build of Live only natively recognizes plugins compiled for Apple Silicon.
status: current
supersedes: []
tags: [subject:ableton-live-plugin-formats, domain:audio, tech:ableton-live, tech:vst3, phase:research]
applies_to_version: Ableton Live 12 (VST3 since 10.1; AUv3 since 11.2)
source: { url: "https://help.ableton.com/hc/en-us/articles/5937501570460-Supported-Plug-in-Formats", doi: "", title: "Supported Plug-in Formats", locator: "Ableton Help Center table of supported formats", accessed: "2026-10-27", tier: 1 }
confidence: verified
derived_from: [C-002]
discovered_by: planner
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-27T12:00:00Z
---
Corroborated by two further Ableton help pages (macOS and Windows plug-in guides). Caveat: AUv3 unsupported on macOS 10.13–10.14; irrelevant to this project's macOS 11+ floor.
