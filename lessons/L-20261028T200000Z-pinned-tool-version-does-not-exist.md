---
id: L-20261028T200000Z-pinned-tool-version-does-not-exist
title: The plan pinned pluginval 2.0.0 and the action tag v2, neither of which exists; CI failed before testing anything
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:pluginval, tech:github-actions, phase:planning, kind:wrong-assumption]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-28T20:00:00Z
---
## Trigger
A plan or workflow pins a tool or action version by name (`@v2`, "2.0.0") that was never looked up against the project's published tags and releases.

## What went wrong
The first CI run failed at job setup: "Unable to resolve action `tracktion/pluginval@v2`". The repository has no `v2` tag and no stable 2.0.0; only `v2.0.0-beta1..3` and `v1.0.x`. Once the action resolved, its default binary (`v2.0.0-beta1`) aborted on macOS with a missing `librtcheck.dylib`, a second pin that had to be set separately.

## Correction
Pin the action ref and its `pluginval-version` input to an existing release, and record the deviation from the plan's pin.

## Prevention rule
Resolve every pinned version against the upstream tag/release list (`gh api repos/<o>/<r>/releases`, `git ls-remote --tags`) when writing the plan, and check the defaults of any wrapper action that downloads a second artifact.

## Detection check
`gh api repos/<owner>/<repo>/git/ref/tags/<tag>` returns 200 for every pin.

## Evidence
guitar-amp-vst D-001i; run 40759996906 (unresolvable tag), run 40763261991 (beta1 abort), green from run 40771992301.
