# ASSUMPTIONS — Intake Records (A-###)

Phase 0.3.6 intake batch. The human was presented with the ambiguities below; unanswered items adopt the proposed default, recorded here.

## A-001 — Repository access
**Ambiguity:** `github.com/qualitycoding/guitar-amp-vst` returned HTTP 404 during planning (private or not yet created). Push access could not be verified; per Phase 0.1 the planning agent halts on missing push access.
**Default adopted:** Plan artifacts are persisted locally as a complete generation-branch tree (`guitar-amp-vst-plan/`) for the human to push as branch `gen-20261027T120000Z-guitar-amp-vst`. The implementer (S-000) re-verifies push access and halts with `BLOCKED.md` if absent.
**Consequence if wrong:** If the repo is private and readable, the human pushes this tree as-is; nothing else changes.

## A-002 — Target platforms
**Ambiguity:** Which OSes to support.
**Default adopted:** Windows 10+ (x64) and macOS 11+ (Universal: Apple Silicon + Intel). Linux is out of scope.
**Consequence if wrong:** Adding Linux later costs one CI matrix row and one packaging step; removing a platform shrinks CI.

## A-003 — Plugin formats
**Ambiguity:** VST3 / AU / VST2 / AAX / CLAP.
**Default adopted:** **VST3** (primary; Ableton Live 12 supports 64-bit VST3 on Windows and macOS), **AUv2** (macOS convenience), **Standalone** (testing without a DAW). VST2 excluded (Steinberg no longer issues new VST2 licenses; legacy format). AAX excluded (requires Avid agreement; user uses Live, not Pro Tools). CLAP excluded from v1 (Live does not host CLAP natively).
**Consequence if wrong:** JUCE can add CLAP/AAX targets later without DSP changes.

## A-004 — Framework and DSP approach
**Ambiguity:** JUCE vs iPlug2 vs custom; circuit modeling (WDF) vs waveshaping vs neural (NAM/LSTM).
**Default adopted:** **JUCE 8.0.12 + CMake**, DSP via **oversampled staged waveshaping with per-voicing pre/de-emphasis filtering** plus **cabinet IR convolution**. Neural captures (NAM/LSTM) rejected for v1: training pipeline, model licensing, and CPU cost exceed the budget of a first release; WDF full-circuit modeling rejected: per-topology development time is too high for 7 voicings.
**Consequence if wrong:** The chain is modular; a neural stage can replace the preamp stage behind the same interface later.

## A-005 — Amp voicings and genre coverage
**Ambiguity:** Exact amp models.
**Default adopted (7 voicings):** `Tweed` (50s low-gain, blues), `Plexi` (late-60s/70s blues rock & classic rock), `JCM800` (80s hard rock/hair metal), `Recto` (90s–00s metal), `5150` (modern high gain, death metal), `Chainsaw` (HM-2-style Swedish death/black), `Frost` (boosted, scooped, tight black metal and beyond). Voicings are parameterizations of one engine (D-002), not 7 separate circuits.
**Consequence if wrong:** A voicing is a data table + filter coefficients; adding/removing one is a small change.

## A-006 — Preset inventory
**Ambiguity:** Number and names of factory presets.
**Default adopted:** 24 factory presets across genres: 70s blues rock, southern rock, classic rock, 80s hair metal, thrash, 90s groove/nu-metal, death metal, Swedish death, black metal, doom/sludge, djent/modern prog, plus clean/crunch utilities. Presets are full APVTS state snapshots (XML), factory ones embedded via BinaryData, user presets in the OS app-data dir.
**Consequence if wrong:** Presets are data; inventory changes don't touch code.

## A-007 — Automation scope
**Ambiguity:** "All controls automatable" — literal?
**Default adopted:** **Literal.** Every user-facing control is an APVTS parameter (float, bool, or choice), exposed to the host with stable IDs, smoothed against zipper noise (D-006), with correct begin/end change gestures (D-007). The only non-automatable items are preset *browser* actions (load next/previous), which are UI-only.
**Consequence if wrong:** none — this is the strict reading and matches the request.

## A-008 — Knowledge store (amended 2026-10-27)
**Ambiguity:** Where lessons/knowledge persist (Rule 13).
**Original default (superseded):** A repo named `agent-knowledge` owned by `qualitycoding` (protocol default proposal).
**Human correction (intake answer):** The store is **`github.com/qualitycoding/knowledge`**. Read access from the planning environment returned HTTP 404 (private or not yet created — indistinguishable without credentials), so R0 loaded nothing and the absence is recorded per Phase 0.3.7; S-000 re-verifies read access from the implementer's authenticated environment (decision rule DR-9), and S-KNOW pushes to this store after gate G-003 (creating it with the Appendix C layout only if it truly does not exist).
**Consequence if wrong:** G-003 allows `push-to: <target>`.

## A-009 — Licensing
**Ambiguity:** Code license.
**Default adopted:** GPL-3.0 for the repo (JUCE's free tier requires GPL; commercial JUCE license is a future purchasable option). Bundled cabinet IRs must be original captures or CC0; this is a release blocker verified in S-016 (R-006 mitigation).
**Consequence if wrong:** Buying a JUCE commercial license changes only the license file and CI secrets.

## A-010 — Performance targets
**Ambiguity:** CPU/latency budgets.
**Default adopted:** ≤ 15% of one core (i5-class, 2019+) per instance at 48 kHz / 256-sample buffer at "High" oversampling; zero added latency at "Eco" (no oversampling) mode; ≤ 3 ms at "High". Sample rates 44.1–192 kHz. Mono→stereo and stereo→stereo buses.
**Consequence if wrong:** S-013 has an explicit decision rule (DR-3) if targets are missed.

## A-011 — GUI style
**Ambiguity:** Photorealistic skeuomorphic amp face vs flat modern.
**Default adopted:** Flat modern dark UI with amp-style rotary knobs (custom LookAndFeel, vector-drawn, no raster assets), resizable via corner drag. Resizability keeps it usable at Live's default plugin window sizes.
**Consequence if wrong:** Purely cosmetic (Low severity).

## Software intake (0.3.2) record

- **Target users:** rock/metal guitarists and producers using Ableton Live 12 on Win/mac.
- **Runtime/deployment:** VST3/AU plugin binaries + installers distributed via GitHub Releases.
- **Performance/scale:** A-010.
- **Threat model / data sensitivity:** No network, no telemetry, no credentials; only local file reads (user IRs) and writes (user presets). Data sensitivity: none.
- **Data handling/retention:** User presets and IRs stay on the user's disk; nothing leaves the machine.
- **Release channel / versioning:** GitHub Releases; SemVer (`v1.0.0` first public release).
- **Maintenance:** Issues on GitHub; no SLA; bus-factor-of-one acknowledged as risk R-010.
