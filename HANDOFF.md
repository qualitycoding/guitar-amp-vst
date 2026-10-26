# HANDOFF — Rock & Metal Guitar Amp VST ("GrimAmp" working title)

**Run ID:** `run-20261027T120000Z-guitar-amp-vst`
**Generation branch:** `gen-20261027T120000Z-guitar-amp-vst`
**Target repo:** `github.com/qualitycoding/guitar-amp-vst` (currently private/absent — see `plan/ASSUMPTIONS.md` A-001)

## Purpose

Build a rock-and-metal guitar amp simulator audio plugin spanning 70s blues rock to high-gain black metal and beyond, with genre-specific factory presets plus full manual dial control, where **every control is automatable in Ableton Live 12** (VST3, 64-bit, Windows + macOS).

## Active profiles

- `software` (with `software.deploys = true` — a distributed plugin artifact is released)
- `math`, `computational`, `publication`: **N/A**

## Reading order

1. `plan/PROFILE.md` — profiles, mode flags, N/A sections
2. `plan/ASSUMPTIONS.md` — every intake default adopted (A-###)
3. `plan/DECISIONS.md` — locked design decisions + if→then decision rules (D-###)
4. `plan/ENVIRONMENT.md` — pinned toolchain, verified setup commands
5. `tests/TEST_SPEC.md` — frozen tests (T-###); verify hashes with `tests/FROZEN_MANIFEST.sha256`
6. `plan/PLAN.md` — the steps, S-000 → S-RETRO → S-KNOW
7. `plan/GATES.md` — human gates
8. `plan/OPERATIONS.md` — release runbook and rollback
9. `premortem/RISK_REGISTER.md` — risks and the mitigations baked into the plan

## Environment setup

Run `plan/ENVIRONMENT.md` top to bottom. It is literal: clone, CMake configure, build, run the frozen suite. Pinned: JUCE 8.0.12, CMake ≥ 3.25, C++20, pluginval 2.0.0, GitHub Actions `windows-latest` + `macos-latest`.

## Running the frozen suite

```bash
sha256sum -c tests/FROZEN_MANIFEST.sha256   # verify the freeze first
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
ctest --test-dir build --output-on-failure  # unit + DSP tests
pluginval validate --strictness-level 5 build/GrimAmp_artefacts/Release/VST3/GrimAmp.vst3
```

## Step list at a glance

S-000 environment/access verification → S-001 repo & CI scaffold → S-002 JUCE skeleton builds and passes pluginval → S-003 parameter layout (APVTS) → S-004→S-008 DSP chain (gate/boost, preamp, tone stack, power amp, cab IR) → S-009 amp voicings → S-010 GUI → S-011 preset system + factory presets → S-012 automation hardening → S-013 performance/oversampling → S-014 CI validation → **S-REVIEW / S-ADVERSARIAL / S-MUTATE (code reviews + mutation testing, protocol 3.8, may run concurrently)** → S-015 Ableton Live manual test protocol → S-016 packaging → S-017 release prep (gate G-002) → S-RETRO → S-KNOW (gate G-003).

## Human gates

- **G-002** (Rule 10): before any public release/publish action.
- **G-003** (Rule 10): final step — where to push lessons/knowledge.
- **G-010** (added, Phase 4.4): human tone sign-off in Ableton Live before packaging. See `plan/GATES.md`.

## Halt / deviation protocol

Default rule (restated from Rule 3.2): choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md` with rationale, and continue — **unless** it touches frozen tests, security, data integrity, a public interface, or research integrity, in which case halt and write `BLOCKED.md`. If a frozen test is found invalid, halt and write `TEST_CHALLENGE.md` (Phase 2E.4).

## Integrity rule (Rule 9, restated verbatim)

No step may fabricate, cherry-pick without disclosure, or manually alter data, test results, benchmarks, or figures.

## Rules 11, 12, 13 (restated)

- **Rule 11 (Lessons):** any correction (redo/revert, failed command fixed by a different approach, a check catching your own mistake, a human correction) is recorded as `lessons/L-<YYYYMMDDTHHMMSSZ>-<slug>.md` per Appendix A of the protocol, committed before work continues, with trigger, mistake, correction, one imperative checkable prevention rule, a detection check, and taxonomy tags. A recurrence names `recurrence_of` and strengthens the earlier rule.
- **Rule 12 (Knowledge):** any new fact about the Subject (JUCE, VST3, Ableton Live, DSP techniques, toolchains) is recorded as `knowledge/K-<YYYYMMDDTHHMMSSZ>-<slug>.md` per Appendix B before continuing, with source, locator, version applicability, confidence, and at least one `subject:` tag. Wrong facts are superseded, never edited away.
- **Rule 13 (Knowledge store):** lessons/knowledge persist in the `qualitycoding/knowledge` repo (corrected store location; see A-008 and R-013). Read at research start (R0) — the planner could not read it (404 without credentials), so S-000 re-verifies read access per DR-9 before S-001; written only at the end, after gate G-003. Never store credentials or secrets — redact (e.g. `github_pat_***`).

## Execution log

`S-000` creates `EXECUTION_LOG.md`; every step appends one line per attempt: UTC time, step ID, attempt number, outcome (`pass`/`fail`/`blocked`), commit hash, short note.

## Concurrency

Independent steps may run concurrently (see `Depends on` / `Exclusive resources` per step). While halted at a gate, any step not depending on the gate's outcome may continue — never an external or irreversible action (G-002).

The plan begins with `S-000` and ends with `S-RETRO` then `S-KNOW`. **Do not execute anything external or irreversible without gate sign-off.**
