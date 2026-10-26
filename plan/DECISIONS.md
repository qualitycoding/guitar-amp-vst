# DECISIONS — Locked Decisions (D-###) & Decision Rules (DR-#)

## Locked design decisions

### D-001 — Framework: JUCE 8.0.15 (pinned by tag AND commit hash), CMake, C++20 — amended 2026-10-27
Rationale: mature VST3/AU/Standalone targets from one codebase; APVTS gives host automation for free; largest community knowledge base. **Amendment from R0 knowledge load:** the 8.x line ends at 8.0.15; the 9.x line restructures the VST3 client extension API (`VST3Interface`/`JUCE_VST3_COMPATIBLE_CLASSES`) among 35+ breaking changes (K-20261004T151500Z, verified live 2026-10-27: newest tag 9.0.4). v1 pins 8.0.15 and **forbids silent upgrade to 9.x or to `master`**; the `8.0.16-security` patch tag is evaluated at S-001 (security lens) — if adopted, the hash pin is updated and T-020 re-run. Submodule checkout is verified by commit hash `91ad83ae34a81e0833b1a2b0866f54846370ae53`, not tag name alone. Traces: C-001, C-005, R-014.

### D-002 — One parametric engine, 7 voicings (not 7 circuits)
Signal chain (fixed order, mono-safe stereo):
```
Input Gain → Noise Gate → Boost (TS-style) → Preamp Stage 1 → Preamp Stage 2
→ Tone Stack → Power Amp (saturation + sag) → Cabinet IR Convolution
→ Mic Position/IR Mix → Output Level
```
Each voicing (A-005) = a row in `Source/DSP/Voicings.h`: pre-emphasis filter coefficients, per-stage waveshaper transfer coefficients (tanh-derivative family, D-003), tone-stack center frequencies/Q/gains, power-amp knee/sag constants, default cab IR index. Adding a voicing = adding a row + preset data. Traces: C-006.

### D-003 — Nonlinearity: staged waveshaping under 4x oversampling
Per-stage memoryless waveshaper (`x - a*x^3` / `tanh` blend family) with per-voicing pre/de-emphasis shelving filters, inside a 4x oversampled wrapper (JUCE `dsp::Oversampling`, polyphase IIR). Power-amp sag = envelope-followed bias modulation (one-pole detector, 20 ms attack / 150 ms release), not a physical model. Rejected: full WDF circuit sims (dev time × 7 topologies), neural captures (A-004). Traces: C-006, C-007.

### D-004 — Cabinet: convolution with embedded IRs + user IR loading
8 factory IRs (original or CC0 captures — license verified in S-016), loaded from BinaryData into `dsp::Convolution` (uniform partitioned, non-real-time thread loading for user IRs via `AudioFormatManager`). User IR slot accepts WAV/AIFF, auto-resampled to host rate. Mic position = crossfade between close/off-axis IR pairs (not a physical model).

### D-005 — Parameters: single APVTS, stable string IDs
All controls (A-007) are APVTS parameters declared once in `Source/Parameters.h` (generated list — GUI, presets, and automation all read from it). IDs are kebab-case strings frozen at v1.0.0 and never reused/renamed (automation compatibility across versions). Parameter count target ≈ 30. Choice params (amp voicing, IR select, oversampling mode) are `AudioParameterChoice` — automatable as stepped parameters in Live.

### D-006 — Zipper-noise control: per-parameter smoothing
Every continuous parameter read through `SmoothedValue<float, ValueSmoothingTypes::Multiplicative>` (gain-like) or `::Linear` (EQ/mix-like), 20 ms ramp, reset in `prepareToPlay`. Stepped params (voicing/IR/oversampling) crossfade or apply at block boundary — never mid-block switches that click. Verified by T-010.

### D-007 — Host notification: gestures done right
GUI edits: `beginChangeGesture()` / `setValueNotifyingHost()` / `endChangeGesture()`. Preset loads: parameter values pushed via APVTS `replaceState` — **not** per-param `setValueNotifyingHost`, to avoid hosts (incl. Live) spawning automation tracks on preset change (documented JUCE/VST3 pitfall; see `research/SOURCES.md` S-8). Verified by T-011 in Live.

### D-008 — Presets: XML APVTS snapshots, factory via BinaryData
Factory presets ship as `BinaryData` XML (state = `apvts.copyState()`); user presets save/load as `.grindpreset` (XML) in `~/Library/Application Support/GrimAmp` / `%APPDATA%\GrimAmp`. Preset schema versioned (`version="1"` attr); unknown attrs ignored, missing attrs default (forward/backward tolerance). Round-trip verified by T-008.

### D-009 — Versioning & release: SemVer + GitHub Releases
Tags `vX.Y.Z`; CI builds artifacts on tag; release notes from `CHANGELOG.md`. Rollback per `plan/OPERATIONS.md`.

### D-010 — No copy protection, no telemetry, no network
Offline plugin. `JUCE_USE_CURL=0`, no analytics. Reduces threat model to local file handling (fuzz-tested, T-013).

### D-011 — Mutation testing opted in (added 2026-10-27, protocol v3.4 amendment)
Tool: **Mull 0.34.0 line**, version-matched packages `mull-19` + `clang-19` on `ubuntu-latest` (suffix agreement across package, plugin path, and compiler is the known failure mode — K-20261027T123200Z). Scope: `Source/DSP/*` and `Source/Presets/*` only (DSP numerics and preset parsing are where the frozen suite carries correctness). Score target for T-070: **≥ 75% killed on first run, then the measured baseline becomes the floor for future PRs** (per Mull guidance, gates start at baseline, not aspiration). Opt-out path if the toolchain proves unstable: new `D-###` + `TEST_CHALLENGE.md` for T-070 — never a silent skip.

## Decision rules (if → then) — Rule 3.2

**DR-1 — Dependency/install failure:** If `cmake` configure fails on a pinned version → check `plan/ENVIRONMENT.md` hash pins; retry once with a clean `build/`; if still failing, pin JUCE to the previous patch (8.0.14) and record in `DEVIATIONS.md`. If the JUCE submodule checkout hash ≠ `91ad83ae…` → re-checkout tag 8.0.15; never track `master`, never vendor a copy by hand. If the VST3 SDK submodule is missing → `git submodule update --init --recursive`.

**DR-2 — pluginval failure at strictness 5:** If a validation test fails → reproduce locally with the same pluginval build; fix the plugin (never lower strictness). If the failure is a known pluginval false positive (check pluginval issues), document the issue URL in `DEVIATIONS.md` and add a CI allowlist entry for that single test name only.

**DR-3 — Performance target missed (A-010):** If S-013 measures > 15% core at "High" → first reduce default oversampling to 2x; if still over, move `dsp::Convolution` to its current non-uniform-partitioned config; if still over, gate "High" mode behind a warning label. Never remove the "Eco" zero-latency path.

**DR-4 — Ableton Live manual test failure (S-015):** If a T-0xx item fails in Live → fix in plugin, re-run pluginval, re-run the failing Live item plus T-010/T-011. If Live itself is the defect (confirmed against a second host, e.g. REAPER trial or JUCE AudioPluginHost) → document in `DEVIATIONS.md` with host versions and proceed to gate G-010 with the evidence.

**DR-5 — GUI/layout fork:** If a control cannot be made legible at minimum window size → reduce that section's control grouping; never drop a parameter from the automation list (A-007 is load-bearing).

**DR-6 — License/IP problem with any IR (R-006):** If any factory IR's provenance cannot be documented as original or CC0 → remove that IR, regenerate affected preset IR-mix defaults, re-run T-008/T-009. Never ship an IR with unclear provenance.

**DR-7 — CI/host reachability:** If GitHub Actions or a package host is unreachable → retry with backoff (3×); if persistently unreachable, build locally on both OSes and attach artifacts to the draft release manually; record in `DEVIATIONS.md`.

**DR-9 — Knowledge store re-verification (added with the A-008 amendment):** In S-000, with the implementer's credentials: `gh repo view qualitycoding/knowledge` (or `git ls-remote`). If **readable** → clone it, run the Appendix C matching rule against this task's tags (`domain:audio`, `tech:juce`, `tech:vst3`, `tech:ableton-live`, `tech:cmake`, `tech:github-actions`, `tech:pluginval`, plus `applies:all`), record the loaded entries in `research/PRIOR_KNOWLEDGE.md` (amendment logged in `DEVIATIONS.md`), and apply each lesson's prevention rule to the matching step's "Lessons applied" field before starting S-001. If any loaded lesson invalidates a frozen test or a locked decision → halt with `TEST_CHALLENGE.md` / `BLOCKED.md`. If **not readable/absent** → record `readable: false` in `state.json` and continue unchanged; S-KNOW creates the store after G-003 only if it truly does not exist.

**DR-8 — Default rule (unanticipated situations), restated verbatim from the protocol:**
> Choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md` with rationale, and continue — **unless** it touches frozen tests, security, data integrity, a public interface, research integrity, or (upward) a statement's evidence class, in which case halt and write `BLOCKED.md`.
