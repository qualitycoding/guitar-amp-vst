---
id: L-20261101T140000Z-intermittent-host-failure-needs-a-stress-test
title: An intermittent pluginval failure was only diagnosable once a host-style automation stress test reproduced it in CI
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:juce, tech:vst3, phase:verification, kind:verification-gap]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-11-01T14:00:00Z
---
## Trigger
pluginval fails with "An unhandled exception was thrown!" in its Automation test on some runs and passes on others, after a change that moves work between threads.

## What went wrong
Moving convolution IR loads to a worker thread passed every unit test and the allocation probe, then pluginval's Automation test failed once on macOS and once on Windows with no message. Re-running gave a pass, which hides the problem. Guessing at the cause cost several CI cycles.

## Correction
Wrote `automation_stress` (random automation of every parameter in 32-sample sub-blocks, several rates and block sizes, re-prepare mid-run) and ran it in the ASan/UBSan job with the exception message surfaced. It reproduced `std::bad_function_call` deterministically; the same test passed after reverting the worker.

## Prevention rule
For any change that touches threading or the audio path, add or run a host-style stress test (random parameter automation at several block sizes, re-prepare mid-run) in CI before merging; treat an intermittent pluginval failure as a reproducible bug to be captured by such a test, not as a flake.

## Detection check
`automation_stress` passes in the sanitizer job on the branch before and after the change.

## Evidence
guitar-amp-vst commits 7b7ea52 (worker, failing), 7b32c8b (stress test reproduces), 2ab8a57 (revert, passes); D-001t.
