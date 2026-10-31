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
        haveGains = false;
    }

    void reset()
    {
        lowShelf.reset();
        highShelf.reset();
    }

    void processBlock (juce::dsp::AudioBlock<float>& block, float lowShelfDb, float highShelfDb)
    {
        // CR-005: coefficients are rebuilt (heap allocation) only when a gain actually changed
        if (! haveGains || lowShelfDb != lastLowDb || highShelfDb != lastHighDb)
        {
            *lowShelf.state  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
                sampleRate, 120.0, 0.7071f, juce::Decibels::decibelsToGain (lowShelfDb));
            *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                sampleRate, 4500.0, 0.7071f, juce::Decibels::decibelsToGain (highShelfDb));
            lastLowDb = lowShelfDb;
            lastHighDb = highShelfDb;
            haveGains = true;
        }

        lowShelf.process (juce::dsp::ProcessContextReplacing<float> (block));
        highShelf.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

private:
    double sampleRate = 44100.0;
    float lastLowDb = 0.0f, lastHighDb = 0.0f;
    bool haveGains = false;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> lowShelf, highShelf;
};

} // namespace gramp
