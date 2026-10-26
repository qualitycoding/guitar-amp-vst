# Research Round 1 — 2026-10-27

## R0 (knowledge store load) — amended 2026-10-27
Store corrected by the human to `qualitycoding/knowledge` (A-008 amendment; lesson L-20261027T121000Z-wrong-knowledge-store). Access attempt returned HTTP 404 — private or nonexistent, indistinguishable without credentials → nothing loaded, Medium risk R-013 recorded, re-verification assigned to S-000 via DR-9. Recorded in `.checkpoints/state.json` (`knowledge_store.readable = false`). `research/PRIOR_KNOWLEDGE.md` documents this.

## Objective
Answer the load-bearing questions in `research/QUESTIONS.md` (Q1 host/format, Q2 toolchain, Q3 DSP approach, Q4 automation).

## Actions
1. Located the planning protocol itself (`qualitycoding/skills`, `protocols/planning-protocol.md`, v3.3) and read it in full.
2. Checked the target repo: `github.com/qualitycoding/guitar-amp-vst` → HTTP 404 via web and GitHub API → A-001 (halt-on-push-access rule satisfied by delivering the branch tree locally for the human to push).
3. JUCE version check → 8.0.12 (C-001). Noted it ships VST3 SDK 3.8.0 under MIT — relevant to A-009 licensing.
4. Ableton format support → official help docs (C-002). Noted the Apple Silicon Universal-build requirement → CI builds `macos-latest` universal binaries.
5. Validation tooling → pluginval 2.0.0 (C-003); strictness 5 chosen as the CI gate (project guidance suggests ≥ 5 for meaningful coverage incl. Steinberg validator).
6. Automation semantics → APVTS docs (C-004) + a JUCE forum thread on preset-load automation spam (C-008) → drove D-007 and tests T-011/T-032. This was the single most consequential research find: without it, "load a preset in Live" would spray automation lanes.
7. DSP approach → Q3 decision recorded (C-006 adopted, C-007 single-source deferral of neural/WDF).

## Findings → artifacts
- C-001…C-008 → `research/claims.json`
- C-002 → format decision A-003 (VST3 primary)
- C-008 → D-007, T-011, T-032, S-012
- All → `research/SOURCES.md`

## Stopping condition
Every load-bearing question answered with ≥ corroborated confidence except C-007 (single-source, flagged, pre-mortemed as R-002). Round closed.
