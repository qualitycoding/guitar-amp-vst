/*
  PostEq.h — optional output low/high shelves (post-eq-low / post-eq-high).
  Coefficients recalculated per block from the smoothed gain values (D-006);
  filter state persists across blocks for zipper-free gain changes.
*/
#pragma once

#include <juce_dsp/juce_dsp.h>

namespace gramp
{
class PostEq
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        lowShelf.prepare (spec);
        highShelf.prepare (spec);
        sampleRate = spec.sampleRate;
    }

    void reset()
    {
        lowShelf.reset();
        highShelf.reset();
    }

    void processBlock (juce::dsp::AudioBlock<float>& block, float lowShelfDb, float highShelfDb)
    {
        *lowShelf.state  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sampleRate, 120.0, 0.7071f, juce::Decibels::decibelsToGain (lowShelfDb));
        *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, 4500.0, 0.7071f, juce::Decibels::decibelsToGain (highShelfDb));

        lowShelf.process (juce::dsp::ProcessContextReplacing<float> (block));
        highShelf.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

private:
    double sampleRate = 44100.0;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> lowShelf, highShelf;
};

} // namespace gramp
