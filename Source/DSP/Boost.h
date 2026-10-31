/*
  Boost.h — TS-style boost stage (S-004).

  Input high-pass ~720 Hz, symmetric soft clip (tanh), output low-pass; the
  boost amount blends the dry and clipped signals and scales the drive into
  the clipper. Bypassed entirely when boostOn is false (processor-level).
*/
#pragma once

#include <juce_dsp/juce_dsp.h>

namespace gramp
{
class Boost
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        *highPass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (
            sampleRate, 720.0, 0.7071f);
        *lowPass.state  = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
            sampleRate, 4500.0, 0.7071f);

        highPass.prepare (spec);
        lowPass.prepare (spec);
        reset();
    }

    void reset()
    {
        highPass.reset();
        lowPass.reset();
    }

    void setAmount (float newAmount) noexcept { amount = juce::jlimit (0.0f, 1.0f, newAmount); }

    void processBlock (juce::dsp::AudioBlock<float>& block) noexcept
    {
        highPass.process (juce::dsp::ProcessContextReplacing<float> (block));

        const float driveGain = 1.0f + 8.0f * amount;
        const float dry = 1.0f - 0.7f * amount;
        const float wet = 0.9f * amount;

        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* d = block.getChannelPointer (ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                const float x = d[i];
                const float clipped = std::tanh (x * driveGain);
                d[i] = dry * x + wet * clipped;
            }
        }

        lowPass.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

private:
    double sampleRate = 0.0;
    float amount = 0.5f;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> highPass, lowPass;
};

} // namespace gramp
