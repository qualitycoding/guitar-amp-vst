# SOURCES — Bibliography with Verification Notes

| ID | Source | Locator | Used for | Verification |
|---|---|---|---|---|
| S-1 | JUCE CHANGE_LIST.md, master branch | github.com/juce-framework/JUCE/blob/master/CHANGE_LIST.md | C-001 (JUCE 8.0.12 current; VST3 SDK 3.8.0 MIT) | Read directly during planning; version list shows 8.0.12 as latest entry. Tier 1 (official repo). |
| S-2 | Ableton Help: "Supported Plug-in Formats" | help.ableton.com/hc/en-us/articles/5937501570460 | C-002 (VST3 64-bit as of Live 10.1; AUv2/AUv3 macOS; no AAX) | Official vendor doc. Tier 1. |
| S-3 | Ableton Help: "Using AU and VST plug-ins on macOS" | help.ableton.com/hc/en-us/articles/209068929 | C-002 corroboration; Apple Silicon note (Universal builds needed) | Official vendor doc. Tier 1. |
| S-4 | Ableton Help: "Using VST plug-ins on Windows" | help.ableton.com/hc/en-us/articles/209071729 | C-002 corroboration; VST3 install path `C:\Program Files\Common Files\VST3\` | Official vendor doc. Tier 1. |
| S-5 | pluginval repo (Tracktion), incl. CLAUDE.md project guide | github.com/Tracktion/pluginval | C-003 (v2.0.0; strictness levels; CLI `validate --strictness-level 5`; CI composite actions; integrates auval/VST3 validator) | Read directly. Tier 1 (official repo). |
| S-6 | JUCE tutorial: "Saving and loading your plug-in state" (APVTS) | juce.com/tutorials/tutorial_audio_processor_value_tree_state/ | C-004 (APVTS parameter model, `getRawParameterValue`, gain-ramp smoothing idiom) | Official framework docs. Tier 1. |
| S-7 | JUCE forum: "VST3 parameter updates: automation vs. host refresh" (Oct–Nov 2025 thread) | forum.juce.com/t/vst3-parameter-updates-automation-vs-host-refresh/67373 | C-008 (preset loads via per-param `setValueNotifyingHost`/`performEdit` can spawn unwanted automation tracks in some hosts; Live mentioned; use `beginChangeGesture` discipline and `replaceState` for preset loads) | Community thread w/ experienced devs; treated as corroborated, not verified → D-007 + T-011/T-032 test it directly. Tier 2. |
| S-8 | GuitarML SmartAmpPro repo (neural amp sim precedent) | github.com/GuitarML/SmartAmpPro | C-007 context (LSTM capture approach exists and works, but adds training pipeline) | Skimmed. Tier 2. Used only to justify deferral, not as a correctness claim. |

## Verification notes

- All host-format claims (C-002) come from first-party Ableton documentation — two independent help pages agree.
- The one **single-source** claim is C-007 (planner's domain judgment that neural/WDF exceeds v1 budget); the pre-mortem (R-002) examines what happens if this judgment is wrong, and D-002's modular chain keeps the migration path open.
- No sources required for D-002/D-003 DSP topology beyond standard practice; the acceptance of tone is delegated to human gate G-010 with rendered audio evidence, not to literature claims.
