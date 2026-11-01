/*
  Voicings.h — one parametric engine, 7 voicings (D-002, A-005).

  A voicing is a data row: per-stage pre-emphasis shelving filters, waveshaper
  coefficients (tanh / cubic blend family, D-003), tone-stack center
  frequencies and gain ranges, power-amp knee/asymmetry/sag constants, a
  loudness-compensation gain (calibrated in S-009) and a default cabinet.

  loudnessGainDb values were calibrated by rendering a 10 s DI reference clip
  through each voicing at matched dial positions and matching integrated
  loudness within +/-1.5 dB (see validation/renders and tests/loudness_spread.py).
*/
#pragma once

namespace gramp
{
struct VoicingRow
{
    const char* name;

    // Preamp stage 1: pre-emphasis high shelf -> shaper -> de-emphasis
    float s1PreFreq, s1PreGainDb;
    float s1Drive;          // shaper input gain at drive knob = 1 (per-stage fraction)
    float s1OutGainDb;

    // Preamp stage 2
    float s2PreFreq, s2PreGainDb;
    float s2Drive;
    float s2OutGainDb;

    // Shaper blend: 0 = cubic-heavy (x - x^3/3), 1 = pure tanh
    float tanhBlend;

    // Tone stack centers (Hz) and knob ranges (dB, applied +/-range/2 around 0.5)
    float bassFreq,   bassRangeDb;
    float midFreq,    midRangeDb;
    float trebleFreq, trebleRangeDb;
    float presenceFreq, presenceRangeDb;   // high shelf, 0 dB at knob 0
    float depthFreq,    depthRangeDb;      // low shelf, 0 dB at knob 0

    // Power amp
    float powerKnee;        // drive into the output-tube saturator
    float powerAsym;        // even-harmonic (2nd) blend 0..1
    float sagAmount;        // envelope-followed bias modulation at sag knob = 1

    // Calibration
    float loudnessGainDb;   // per-voicing loudness compensation (S-009)

    int   defaultCab;       // factory cab index (A-005/D-004)
};

inline constexpr VoicingRow voicings[] =
{
    // name       s1:  preF   preG   drive  outG   s2:  preF   preG   drive  outG   tanhB
    //            bassF  bassR  midF   midR   trebF  trebR  presF  presR  depthF depthR
    //            knee   asym   sag    loudG  cab
    {
        "Tweed",
         2200.0f,  5.0f,  1.6f,  -2.0f,   3200.0f,  4.0f,  1.2f,  -1.5f,   0.25f,
         150.0f, 14.0f,  420.0f,  9.0f,  2200.0f, 12.0f,  4200.0f,  6.0f,  120.0f, 6.0f,
         1.10f,  0.55f,  0.25f,  +1.7f,   3
    },
    {
        "Plexi",
         2600.0f,  6.0f,  2.1f,  -2.5f,   3800.0f,  5.0f,  1.6f,  -2.0f,   0.40f,
         120.0f, 12.0f,  650.0f, 10.0f,  2600.0f, 13.0f,  4800.0f,  7.0f,  110.0f, 6.0f,
         1.25f,  0.45f,  0.30f,  +0.9f,   0
    },
    {
        "JCM800",
         3000.0f,  7.0f,  2.5f,  -3.0f,   4200.0f,  6.0f,  2.0f,  -2.5f,   0.55f,
         110.0f, 12.0f,  800.0f, 10.0f,  2900.0f, 13.0f,  5200.0f,  8.0f,  100.0f, 6.0f,
         1.35f,  0.40f,  0.35f,  +0.4f,   0
    },
    {
        "Recto",
         1800.0f,  8.0f,  3.0f,  -3.5f,   2400.0f,  7.0f,  2.6f,  -3.0f,   0.70f,
          90.0f, 13.0f,  450.0f,  9.0f,  2400.0f, 14.0f,  5000.0f,  8.0f,   90.0f, 7.0f,
         1.50f,  0.30f,  0.40f,  -0.5f,   1
    },
    {
        "5150",
         2000.0f,  9.0f,  3.4f,  -4.0f,   2600.0f,  8.0f,  3.0f,  -3.5f,   0.80f,
          85.0f, 13.0f,  400.0f,  8.0f,  2500.0f, 14.0f,  5200.0f,  8.0f,   85.0f, 7.0f,
         1.60f,  0.25f,  0.45f,  -0.7f,   1
    },
    {
        "Chainsaw",
         1500.0f, 10.0f,  3.8f,  -4.0f,   2000.0f,  9.0f,  3.4f,  -3.5f,   0.90f,
         110.0f, 14.0f, 1000.0f, 16.0f,  1800.0f, 15.0f,  4800.0f,  9.0f,  120.0f, 8.0f,
         1.70f,  0.35f,  0.50f,  -1.2f,   6
    },
    {
        "Frost",
         2400.0f,  9.0f,  4.2f,  -4.5f,   3000.0f,  9.0f,  3.8f,  -4.0f,   1.00f,
          80.0f, 12.0f,  350.0f, 12.0f,  3200.0f, 15.0f,  5600.0f,  9.0f,   80.0f, 7.0f,
         1.80f,  0.20f,  0.55f,  -1.3f,   7
    },
};

inline constexpr int numVoicings = (int) (sizeof (voicings) / sizeof (voicings[0]));
static_assert (numVoicings == 7, "A-005 fixes the voicing count at 7");

} // namespace gramp
