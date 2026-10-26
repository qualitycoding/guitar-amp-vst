# QUESTIONS — Research Question Tree

## Q1. What host/format constraints does Ableton Live impose? [LOAD-BEARING]
- Q1.1 Which plugin formats does Live 12 support on Win/mac? → **answered:** VST2/VST3 64-bit both OSes; AU macOS-only; VST3 since Live 10.1. (C-002, verified)
- Q1.2 Any Live-specific automation gotchas? → **answered:** preset-change notification can spawn unwanted automation tracks in some hosts incl. Live; use replaceState for preset loads, gestures for GUI edits. (C-008, corroborated)

## Q2. What framework/toolchain? [LOAD-BEARING]
- Q2.1 Current JUCE version and its bundled SDKs? → **answered:** 8.0.12, VST3 SDK 3.8.0 (MIT). (C-001, verified)
- Q2.2 Validation tooling? → **answered:** pluginval 2.0.0 (Tracktion), strictness levels, CLI + GitHub Actions, integrates auval/vst3validator. (C-003, verified)

## Q3. What DSP approach covers blues-rock → black metal within a solo-dev budget? [LOAD-BEARING]
- Q3.1 WDF circuit modeling? → rejected for v1 (per-topology cost × 7). 
- Q3.2 Neural captures (NAM/LSTM)? → rejected for v1 (training pipeline, licensing, CPU). (C-007, single-source — based on planner's domain assessment; revisited in pre-mortem R-002)
- Q3.3 Staged waveshaping + IR convolution? → **adopted** (D-002/D-003/D-004). Industry-standard approach for non-neural amp sims. (C-006, corroborated)

## Q4. How is parameter automation implemented so every dial is automatable and click-free?
- **answered:** APVTS for all params (D-005), SmoothedValue ramps (D-006), attachment-based GUI binding + gesture discipline (D-007). (C-004, verified; C-005, corroborated)

## Q5. What genre presets and voicings?
- **answered by intake defaults:** A-005 (7 voicings), A-006 (24 presets). Tone adequacy is a human judgment → gate G-010 exists (R-001 mitigation).

## Open questions (none blocking; resolved by defaults)
- OQ-1: Exact IR capture sessions (mic/cab choices) — resolved at S-016 by the human's captures or CC0 library selection; provenance is the only hard requirement (DR-6).
- OQ-2: Final product name ("GrimAmp" is a working title; rename before S-016, Low severity — see R-011).
