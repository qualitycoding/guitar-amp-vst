# G-010 research round: why the candidates lack grit and weight

Date: 2026-11-04. Trigger: three scored listening rounds (9/12, 11/12, 7/12) and the listener's notes: "more grit" (round 1) and "more grit and more weight" (round 2).

## Sources

- Pakarinen & Yeh, "A Review of Digital Techniques for Modeling Vacuum-Tube Guitar Amplifiers", Computer Music Journal 33(2), 2009 (read in full text, pp. 85-100).
- Karjalainen et al. 2006 and Kuroki & Ito 1998, as summarised in that review: dynamic preamp models.
- Pritchard 1991, as summarised there: two distortion blocks with an equaliser between them; first block high-passed and asymmetric.
- Yeh thesis (Stanford, 2009): table of contents only was usable; no numbers taken from it.
- Marshall JVM 410H dataset (Zenodo 7970723): descriptive page only; the DAFx23 paper text was not retrieved.
- `qualitycoding/knowledge` store: no item on gain structure (it holds web-audio, transcription and pedal-note items).

## What the literature says a preamp stage does that ours did not

1. A preamp is several triode stages in cascade, each with a low-pass (grid resistor and Miller capacitance) and a high-pass (interstage DC-blocking capacitor), and sign inversion between stages.
2. The stage is not memoryless. Grid current charges the coupling capacitor and the cathode bypass capacitor holds the bias, so the operating point follows the signal envelope and relaxes afterwards ("blocking distortion", dynamic bias). The review states that the stages therefore cannot be collapsed into one lookup table.
3. Clipping is asymmetric, which creates even as well as odd harmonics. Symmetric shaping gives odd harmonics only.
4. Large transients and sag compress the dynamics of the power stage.

## What our preamp did

Each stage: high shelf, gain, symmetric memoryless shaper (tanh/cubic blend), shelf. No bias, no envelope dependence, symmetric clipping.

## Hypotheses tested on the 14 reference pairs (candidate minus reference, level-matched)

| Hypothesis | Result |
|---|---|
| Tone stack position is the problem | Rejected. Marshall-style preamps clip before the tone stack, as ours does; bypassing tone stack or power amp changes 2-8 kHz by 1-2 dB. |
| Shaper knee too soft | Rejected. Harder knee moved the gap under 0.5 dB. |
| More pre-emphasis | Rejected. 60 % more changed it under 1 dB. |
| Split-band / coupling high-pass (150 Hz) | Raised high-frequency share but cost 5 dB of 80-250 Hz weight; round 2 scored 11/12. Reverted (D-001z). |
| Stage bias, static plus envelope-driven, with DC block | Accepted (D-001aa). Crest factor 6.9 dB to 10.4 dB (references 11.1); 80-250 Hz gap -3.3 to -0.8 dB; 2-8 kHz changed by less than 1.5 dB. |

Bias values were swept over static 0 to 0.8 and dynamic 0 to 2.0. Larger values overshoot the references' crest factor (14-22 dB), so 0.4 / 0.5 was taken as the point that matches the references' crest factor.

## What this does not fix

The 4-16 kHz band is still about 8 dB under the references on average, and 15-35 dB under for the low-drive presets in the earlier measurement. The research does not offer a cheap lever for it: it points to real stage-by-stage gain structure (several stages each clipping a differently filtered signal), which would be a larger redesign of the preamp topology than one round can validate.
