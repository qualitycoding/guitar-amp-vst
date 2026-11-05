# GATE G-010 — LISTENING RESPONSE

**Response:** `proceed-with-rescope`  
**Recorded:** 2026-11-02T14:15:19Z  
**Repository:** `qualitycoding/guitar-amp-vst`  
**Branch:** `impl-run-20261027T120000Z-guitar-amp-vst`  
**Candidate commit:** `edd8f90a7886f2e9da67e1f029ef0485153a6c74`  
**Round:** 1 — scored 9/12 correct, one-sided p = 0.0730 (fails the ≤8/12 rule)  
**Test type:** `realism`

The blind-listening amendment has been integrated into the repository plan, but this gate is not ready for a listening round. No reference/candidate bundle has been built, no listener answers exist, and no score or p-value is claimed.

## Missing prerequisites

- S-015 Live record/render evidence is incomplete.
- Unresolved review/host prerequisites must finish first.
- Redistribution-compatible real-amp/reference recordings or an explicit demos-only rescope decision are required.

## Next action

Move final original/CC0 IR selection and candidate render refresh into S-015 before G-010; after proceed, resume S-016 packaging/provenance/installers.

## Allowed future responses

`proceed`, `iterate: <notes>`, `proceed-with-rescope: <text>`, `stop`

## Claim guard

Dry DI is not a realism reference; final tonal content must be judged before release packaging.

## Round 1 outcome (2026-11-03)

Listener (verbatim): "The clean tones were practically indistinguishable but the others all had more grit in the real clip".
Prerequisites PR-001/PR-002 remain open, so this is diagnostic. See `listening/G-010/round-001/result.json` for the measurement and the next seed (20262103).

## Round 2 outcome (2026-11-03)

11/12 correct, one-sided p = 0.0032: worse than round 1 (9/12). Only Bay Area Lead was missed; the two clean presets, indistinguishable in round 1, were identified. One round remains (seed 20263103). See `listening/G-010/round-002/result.json`.

## Round 3 outcome (2026-11-04)

7/12 correct, one-sided p = 0.3872: passes the numeric rule (at most 8/12) and is statistically indistinguishable from chance. No adverse qualitative finding was given. Open: PR-001/PR-002 (S-015 Live evidence, review/host prerequisites); references were partial/weak matches and cover 14 of 24 presets (none for Chainsaw/Frost). Awaiting `proceed`, `iterate`, `proceed-with-rescope` or `stop`.

## Round 4 outcome (2026-11-05, extra round)

11/12 correct, one-sided p = 0.0032. Listener: "These were very difficult to distinguish." Round 3 (D-001z only) scored 7/12; round 4 adds D-001aa. Awaiting decision.

## Round 5 outcome (2026-11-05, extra round)

8/12 correct, one-sided p = 0.1938: passes the numeric rule (at most 8/12), exactly at the limit, not significantly different from chance. No qualitative notes given. Open: PR-001/PR-002; references partial; no references for Chainsaw/Frost. Awaiting `proceed`, `iterate`, `proceed-with-rescope` or `stop`.

## Response: proceed-with-rescope (2026-11-05)

Human response: `proceed-with-rescope` (recommended option accepted).

**Narrowed claim.** Realism against real-amp recordings is claimed only for the Marshall-family presets that had a close or partial reference (JCM800 and Plexi presets, and loosely the 5150 presets), judged on 1 s onset windows of the amp output without a cabinet. The 5150 presets are judged only as "a real high-gain valve amp", not as a 5150. Untested: Chainsaw and Frost (no references), and the Tweed/Recto/Doom presets (their references are CC BY-NC and were not used). The plugin is not claimed to be indistinguishable from real amps: five rounds scored 9, 11, 7, 11 and 8 of 12, and only rounds 3 and 5 meet the 8/12 rule.

**Affected files.** `README.md` (status and claims), `CHANGELOG.md` (v1.0.0 draft), `listening/G-010/round-005/result.json` (rescope block), `DEVIATIONS.md`, `EXECUTION_LOG.md`.

**Still open before S-016 can complete:** PR-001 (S-015 Live evidence), PR-002 (review/host prerequisites), the roughly 6.7 dB loudness drop since round 3 (master make-up gain or an amended T-005 peak bound), a Mull rerun (T-070) after D-001x to D-001ab, and the two rounds beyond the three-round cap.
