---
id: K-20261027T120100Z-juce-preset-automation-spam
statement: In JUCE-built VST3 plugins with an internal preset browser, loading a preset by calling setValueNotifyingHost per parameter emits beginEdit/performEdit-style notifications that some hosts (Ableton Live and Reason reported) record as automation, spawning spurious automation lanes. Preset loads should instead push state through AudioProcessorValueTreeState::replaceState; GUI edits should use beginChangeGesture/setValueNotifyingHost/endChangeGesture.
status: current
supersedes: []
tags: [subject:juce-vst3-preset-automation, domain:audio, tech:juce, tech:vst3, tech:ableton-live, phase:research]
applies_to_version: JUCE 8.x (VST3 wrapper behavior per 2025 forum thread)
source: { url: "https://forum.juce.com/t/vst3-parameter-updates-automation-vs-host-refresh/67373", doi: "", title: "VST3 parameter updates: automation vs. host refresh", locator: "thread Oct-Nov 2025, posts by jpo / otristan / JeffMcClintock", accessed: "2026-10-27", tier: 2 }
confidence: corroborated
derived_from: [C-008]
discovered_by: planner
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-27T12:01:00Z
---
Community-sourced; treated as design-shaping but verified by tests T-011 (mock host gesture audit) and T-032 (in-Live check) rather than trusted outright. Related: hosts display stale parameter values after preset load unless values are pushed — another reason to centralize preset loads on replaceState.
