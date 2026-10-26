# Research Round 2 — 2026-10-27 (v3.4 amendment round)

## Trigger
Protocol upgraded to v3.4 (assumption audit 4.7; code review steps 3.8). The plan is amended to conform, and the 4.7 audit was executed retroactively over the existing plan.

## Actions
1. Verified JUCE 8 licensing against the EULA (S-9) → C-009; **falsified the A-009 premise** (GPL→AGPLv3 + Starter tier). Lesson L-20261027T123000Z and knowledge item K-20261027T123100Z recorded.
2. Verified Mull mutation-testing toolchain (S-10) → C-010; D-011 opts S-MUTATE in with mull-19/clang-19, scope `Source/DSP` + `Source/Presets`, score floor 75%-then-baseline, T-070 frozen via the 2E.4 machinery.
3. Added S-REVIEW / S-ADVERSARIAL / S-MUTATE steps (protocol 3.8) between S-014 and S-015; S-015 now depends on all three so the human tone-tests only post-review code.
4. Remaining audit verdicts in `premortem/ASSUMPTION_AUDIT.md`; no other falsified items; converged in 1 pass.

## Findings → artifacts
- C-009 → A-009 amendment, S-001 LICENSE action (AGPL-3.0)
- C-010 → D-011, S-MUTATE, T-070, ENVIRONMENT.md Mull row
- New risks R-015 (license mismatch — resolved by amendment), R-016 (Mull/LLVM toolchain fragility in CI — opt-out path pre-decided in D-011)
