---
id: L-20261029T020000Z-start-mutation-review-at-lowest-file
title: Review mutation survivors by lowest per-file score first: a 9% file hid a product bug
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:mull, tech:juce, phase:verification, kind:process]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T02:00:00Z
---
## Trigger
A first mutation run scores well under the floor and the report lists dozens of survivors.

## What went wrong
Nothing yet: this records what worked. Per-file scores (Boost 0/9, NoiseGate 3/34) pointed at stages no test exercised. Writing a reference test for NoiseGate immediately exposed that the gate never attenuated (see the decibelsToGain lesson). Chasing individual operator survivors in already-good files would have found nothing.

## Correction
Group survivors by file with a short script over `mull.sqlite` (`execution_status = 2` is survived), start with the worst file, write a behaviour test that asserts the stage's documented effect, and only then justify what remains.

## Prevention rule
Treat a file with a very low mutation score as "this stage may be untested or broken", and write a test from the stage's documentation before reading its mutants one by one.

## Detection check
The per-file table is in the review document and no file under 50% lacks a stated reason.

## Evidence
guitar-amp-vst scores 37.7% (run 40847181602), 68.2% (40867662636), 77.9% (40898109414); reviews/MUTATION.md.
