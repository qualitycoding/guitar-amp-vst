# ASSUMPTION_AUDIT — Phase 4.7 Assumption & Premise Audit (v3.4 amendment)

Run 2026-10-27, fresh-context adversarial stance, after the v3.4 protocol upgrade. Scope: all `A-###`, premises behind `D-###`, and the `single-source`/`inferred` claims in `research/claims.json`.

## Verdicts

| Item | Verdict | Evidence checked | Resulting IDs |
|---|---|---|---|
| A-001 (repo access) | **confirmed** (after in-run correction) | Token-authenticated API: repo exists, public, empty; generation branch pushed (`234c1aa`). | — |
| A-002 (platforms Win+mac) | confirmed-as-default | User's OS never stated; covering both makes the default consequence-free. | — |
| A-003 (formats) | confirmed | C-002 re-verified against Ableton help pages. | — |
| A-004 (framework/DSP approach) | confirmed-with-gate | C-007 remains single-source by nature (engineering judgment); tone adequacy is human-gated (G-010), chain modularity keeps neural/WDF migration open (D-002). | R-001, R-002 |
| A-005 (7 voicings span the genre range) | unverifiable until audio exists | Cannot be checked without a built plugin. | R-001, G-010 (standing mitigation) |
| A-006 (24 presets) | confirmed | Data-only; schema tolerance in D-008. | — |
| A-007 (literal automation scope) | confirmed | Strict reading matches request; D-005/D-006/D-007 implement it. | — |
| A-008 (knowledge store) | **falsified** (original `agent-knowledge`), corrected | Human correction; store read at `09f34582…`. | L-20261027T121000Z-wrong-knowledge-store |
| A-009 (licensing) | **falsified in part** | Re-checked JUCE 8 licensing (juce.com/legal/juce-8-licence): JUCE 8 is dual-licensed under the JUCE licence and **AGPLv3** (not GPLv3), and a free **Starter** tier permits closed-source distribution up to $20k revenue. The premise "the free tier requires GPL" was wrong for JUCE 8. | L-20261027T123000Z-juce-license-assumption; K-20261027T123100Z-juce8-licensing; A-009 amended; S-001 LICENSE becomes AGPL-3.0 |
| A-010 (performance targets) | unverifiable until S-013 | No code exists yet. | DR-3 ladder (standing mitigation) |
| A-011 (GUI style) | confirmed | Cosmetic; Low severity by construction. | — |
| Premise behind D-005/D-007 (APVTS automation; preset-load notification pitfall) | confirmed | C-004 verified (official tutorial); C-008 corroborated + frozen tests T-011/T-032 verify in-host. | — |
| Premise behind D-003 (waveshaper family spans clean→high-gain) | unverifiable by inspection | Same gate as A-005. | R-001/R-002 |
| C-006 (corroborated) | confirmed | Standard architecture; no contradicting evidence found. | — |
| C-007 (single-source) | confirmed-as-judgment | Explicitly an engineering-budget judgment; reviewed in R-002 pre-mortem. | R-002 |

## Iteration

One falsified load-bearing item (A-009): amendment applied in place — A-009 rewritten, S-001 license action updated, lesson + knowledge item recorded. No frozen test depended on the license choice, so no re-freeze was required for it. The v3.4 review steps (S-REVIEW/S-ADVERSARIAL/S-MUTATE) added a frozen check T-070, which went through the 2E.4 machinery: TEST_SPEC amended, manifest regenerated, cold-read items folded in. Re-audit of the changed set: no new falsified items. Converged in 1 pass.
