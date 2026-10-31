---
id: L-20261028T220000Z-decibelstogain-minusinfinity-misuse
title: Passing a linear floor as the second argument of juce::Decibels::decibelsToGain silently disabled the noise gate
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [tech:juce, domain:audio, phase:implementation, kind:wrong-assumption]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-28T22:00:00Z
---
## Trigger
Code calls `juce::Decibels::decibelsToGain (db, X)` with a second argument meant as a minimum linear gain (for example `1.0e-6f`). Signal: a dB-threshold comparison that never fires although the signal is clearly below it.

## What went wrong
The noise gate used `decibelsToGain (thrCloseDb, 1.0e-6f)`. The second parameter is `minusInfinityDb`, not a linear floor: any dB value not above it returns 0. With a positive value every realistic threshold mapped to 0, `envelope < thrLinear` was never true, and the gate passed audio unchanged at every setting. The frozen suite, the loudness calibration and pluginval all passed; it was found only when a mutation-survivor review produced a reference test (a signal 24 dB under the threshold came out unattenuated).

## Correction
Use `decibelsToGain (db)` (default `-100`) or pass a dB value as the second argument; compare against the linear value only after conversion.

## Prevention rule
Read the parameter name of every JUCE helper you pass a literal to; for each stage that is meant to change the signal, keep at least one test that asserts the change (attenuation, boost, filter response), not only "no NaN / no crash".

## Detection check
`grep -rn "decibelsToGain *(.*," Source/` and check each second argument is in dB. A test that feeds a signal below the gate threshold and asserts the output is smaller.

## Evidence
guitar-amp-vst D-001m; `gate_reference` failed on run 40866289187 and passed after the fix on run 40867662636.
