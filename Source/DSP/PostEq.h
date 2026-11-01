/*
  PostEq.h — optional output low/high shelves (post-eq-low / post-eq-high).
  Coefficients recalculated per block from the smoothed gain values (D-006);
  filter state persists across blocks for zipper-free gain changes.
*/
#pragma once

#include <juce_dsp/juce_dsp.h>
#include "BiquadDesign.h"

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
        // size the coefficient objects now (neutral shelves) so processBlock can design in place
        biquad::lowShelf  (*lowShelf.state,  sampleRate, 120.0f,  0.7071f, 1.0f);
        biquad::highShelf (*highShelf.state, sampleRate, 4500.0f, 0.7071f, 1.0f);
    }

    void reset()
    {
        lowShelf.reset();
        highShelf.reset();
    }

    void processBlock (juce::dsp::AudioBlock<float>& block, float lowShelfDb, float highShelfDb)
    {
        // CR-005: coefficients are redesigned (in place) only when a gain actually changed
        if (! haveGains || lowShelfDb != lastLowDb || highShelfDb != lastHighDb)
        {
            biquad::lowShelf  (*lowShelf.state,  sampleRate, 120.0f,  0.7071f, juce::Decibels::decibelsToGain (lowShelfDb));
            biquad::highShelf (*highShelf.state, sampleRate, 4500.0f, 0.7071f, juce::Decibels::decibelsToGain (highShelfDb));
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
