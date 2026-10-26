# Pre-Mortem Round 1 — 2026-10-27

Conducted with fresh context against the full plan tree. One incident report per applicable scenario; lenses per Phase 4.2 (software profile only).

## Incident report A: "The black metal preset is a meme" (tone failure)
Six months post-release, forum consensus: blues/rock voicings are fine, the extreme voicings are thin and fizzy; users say "just use a NAM capture instead." **Root cause:** no automated test can assert "sounds like black metal"; the plan originally trusted DSP metrics (THD, monotonic gain) as a proxy for tone. **Fix applied:** gate G-010 with rendered per-preset audio evidence and an explicit human sign-off; S-009 calibration; pre-decided rescope. Residual: Medium (R-001/R-002).

## Incident report B: "Loading a preset nuked my Live set" (automation spam)
A user arms automation, flips factory presets while writing a track, and Live fills with 30 automation lanes per preset change. **Root cause:** preset loads notified the host per-parameter (the documented JUCE/VST3 `performEdit` behavior; Live among the affected hosts). **Fix applied:** D-007 + frozen tests T-011 and T-032; S-012 audit. Residual: Low (R-003). This was the highest-value pre-mortem find — it converted a forum thread (C-008) into a design rule and two frozen tests.

## Incident report C: "My old sessions sound different" (ID churn)
Beta users' sessions break at v1.0.0 because parameter IDs were renamed during GUI polish. **Fix applied:** ID freeze at S-003, T-002/T-009, rename = BLOCKED. Residual: Low (R-008).

## Incident report D: "Legal pulled the download" (IR licensing)
A factory IR turned out to be ripped from a commercial pack. **Fix applied:** DR-6 + provenance file as release blocker. Residual: Low (R-006).

## Incident report E: "Melting laptops" (performance)
High mode + convolution saturates a core on 2018-era machines; Live's CPU meter pegs, dropouts during tracking. **Fix applied:** DR-3 ladder, Eco mode guaranteed zero-latency, T-050 regression gate. Residual: Low (R-007).

## Lens sweep
- **Dependency drift:** JUCE pinned to tag 8.0.12 via submodule (S-001); pluginval pinned 2.0.0 in CI; DR-1 fallback pin defined. ✔
- **Invalid research assumptions:** every `single-source`/`inferred` claim reviewed → only C-007, covered by R-002. ✔
- **Implementer misinterpretation:** cold-read pass folded items back (gate G-010 evidence list made explicit; "loudness-matched" given a number ±1.5 dB; "click-free" given T-010's 3× bound). ✔
- **Integrity:** no fabricated tone claims — tone is human-gated, not asserted. ✔
- **Security:** no-network design (D-010) + T-040/T-041; fuzz tests T-012/T-013 for the only file inputs. ✔
- **Operational:** R-009/R-010 recorded with runbook and rollback in OPERATIONS.md. ✔

## Verdict
0 Critical / 0 High residual → plan hardened, loop converged at round 1. Full register: `premortem/RISK_REGISTER.md`.
