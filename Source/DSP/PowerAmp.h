/*
  PowerAmp.h — output-tube-style saturator + sag (S-007, D-003).

  Soft-knee saturator with a per-voicing knee and an asymmetry blend that
  injects even (2nd) harmonic content; sag = one-pole envelope follower
  (20 ms attack / 150 ms release, capped at 250 ms by construction) that
  modulates the shaper bias, plus a low-end thump shelf driven by the same
  envelope. The depth knob scales the effect; modulation is capped at +/-20%.
*/
#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Voicings.h"

namespace gramp
{
class PowerAmp
{
public:
    void prepare (double sampleRate, const VoicingRow& row)
    {
        sr = sampleRate;
        knee = row.powerKnee;
        asym = row.powerAsym;
        sagBase = row.sagAmount;

        atk = std::exp (-1.0f / (float) (sampleRate * 0.020));   // 20 ms
        rel = std::exp (-1.0f / (float) (sampleRate * 0.150));   // 150 ms

        *thump.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sampleRate, 110.0, 0.7071f, 1.0f);
        const juce::dsp::ProcessSpec spec { sampleRate, 128, 2 };
        thump.prepare (spec);
        reset();
    }

    void reset()
    {
        env = 0.0f;
        thump.reset();
    }

    void setVoicing (double sampleRate, const VoicingRow& row)
    {
        prepare (sampleRate, row);
    }

    void setSag (float sagKnob) noexcept { sagKnob01 = juce::jlimit (0.0f, 1.0f, sagKnob); }

    /*  Per-sample saturator + sag envelope. Call processThump() afterwards on
        the whole block to apply the low-end shelf driven by the envelope. */
    void process (float* const data, int numSamples) noexcept
    {
        const float depth = sagBase * sagKnob01;
        const float k = juce::jmax (0.1f, knee);

        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];

            // envelope follower on the rectified signal
            const float a = std::abs (x);
            const float c = (a > env) ? atk : rel;
            env = c * env + (1.0f - c) * a;

            // sag: bias modulation capped at +/-20% (S-007 deviation guard)
            const float mod = juce::jlimit (-0.2f, 0.2f, depth * (env - 0.35f));
            const float xs = x * (1.0f - mod);

            // soft-knee saturator with asymmetry (even-harmonic) blend.
            // The even-harmonic term is driven by the INPUT so that silence
            // stays silent (x^2 is zero at zero — a constant-offset term here
            // would leak DC and fail T-004).
            float y = std::tanh (k * xs) / std::tanh (k);
            y = juce::jlimit (-1.5f, 1.5f, y + asym * 0.35f * (xs * xs));

            data[i] = y;
        }
    }

    void processThump (juce::dsp::AudioBlock<float>& block, float sagKnob)
    {
        // low-end thump: low shelf driven by the sag envelope (S-007)
        const float depth = sagBase * juce::jlimit (0.0f, 1.0f, sagKnob);
        const float gainDb = juce::jlimit (0.0f, 6.0f, depth * env * 8.0f);
        *thump.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sr, 110.0, 0.7071f, juce::Decibels::decibelsToGain (gainDb));
        thump.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    float getEnvelope() const noexcept { return env; }

private:
    double sr = 44100.0;
    float knee = 1.2f, asym = 0.3f, sagBase = 0.3f, sagKnob01 = 0.5f;
    float atk = 0.0f, rel = 0.0f;
    float env = 0.0f;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> thump;
};

} // namespace gramp
