# BLOCKED — run-20261027T120000Z-guitar-amp-vst

Exactly what is missing, per plan/S-000. Everything listed here is a **human-gated**
step; no local substitute exists in the build sandbox.

## B-1 — Repository push access (S-000 gate)

- **Missing:** GitHub credentials with push rights to `github.com/qualitycoding/guitar-amp-vst`.
- **Blocks:** pushing branch `impl-run-20261027T120000Z-guitar-amp-vst`, CI
  observation (S-014), all GitHub Actions results (T-020/T-021 pluginval on
  Win/mac, T-070 Mull, T-050 CI artifacts).
- **Local state:** the full implementation + frozen-suite results are committed
  locally on the impl branch; `git push -u origin impl-run-20261027T120000Z-guitar-amp-vst`
  is the only remaining command (plus `git submodule add` push for JUCE if the
  remote doesn't carry it).

## B-2 — Windows + macOS build/validation hosts (T-020/T-021, S-014)

- **Missing:** Windows and macOS machines (or CI minutes after B-1).
- **Blocks:** pluginval strictness 5 on both OSes, CI green runs, T-060 installer
  smoke test, G-002 release.

## B-3 — Ableton Live 12 (T-030..T-034, gate G-010)

- **Missing:** Ableton Live 12 (any edition, 64-bit) with the plugin installed —
  a manual, human-run step by design (S-015).

## B-4 — pluginval on headless Linux (T-020 local half)

- **Missing:** display server/audio session acceptable to pluginval, or a CI run
  after B-1. Attempted locally; recorded in DEVIATIONS.md (D-001d).

## B-5 — CI runner CPU class definition (T-050 verdict)

- **Missing:** plan/ENVIRONMENT.md does not pin the "CI runner class" CPU that
  T-050's 15 % budget references. The number must be read from the first CI run
  after B-1 (GitHub `windows-latest`/`macos-latest` hardware) and the plan amended
  by the human.

## Not blocked (completed locally)

- S-001..S-008 implementation, frozen suite T-001..T-013 + fuzz + bench harness
  (all green), T-050/T-051 measurements (informational, see D-001c).
