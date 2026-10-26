---
id: L-20261027T123000Z-juce-license-assumption
title: Assumed JUCE's free tier requires GPL; JUCE 8 uses AGPLv3 plus a free closed-source Starter tier
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:juce, phase:intake, kind:wrong-assumption]
recorded_by: planner
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-27T12:30:00Z
---
## Trigger
Choosing a repo license for a JUCE project from memory ("JUCE is GPL or paid commercial") without reading the current EULA for the pinned major version.

## What went wrong
A-009 recorded GPL-3.0 as the repo license on the premise that JUCE's free tier requires GPL. The Phase 4.7 audit re-checked: JUCE 8 is dual-licensed under the JUCE licence and **AGPLv3** (network-use clause matters for any future server component), and the free **Starter** tier (≤ $20k revenue, no splash screen) allows closed-source distribution. The premise was stale by a major version.

## Correction
A-009 amended: repo license is AGPL-3.0 if open-sourced (matching JUCE 8's copyleft option), with the Starter tier documented as the free closed-source alternative; S-001's LICENSE action updated.

## Prevention rule
For any framework with a paid tier, read the current EULA/licensing page for the pinned major version before writing the license assumption; never carry a licensing premise across major versions.

## Detection check
`plan/ASSUMPTIONS.md` licensing item names the exact JUCE licence(s) of the pinned version (AGPLv3 / Starter / Indie / Pro for 8.x) and cites the EULA URL with an access date.

## Evidence
juce.com/legal/juce-8-licence (accessed 2026-10-27); JUCE forum EULA announcement (t0m, 2024-04). No secrets.
