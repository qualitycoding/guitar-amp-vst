/*
  ToneStack.h — 3-band tone control + presence + depth (S-006, D-002).

  bass/mid/treble = peaking biquads at per-voicing center frequencies,
  presence = high shelf, depth = low shelf. Coefficients are recalculated
  only when a parameter or the voicing changes (block-rate max); band gains
  themselves arrive smoothed from the processor (D-006).
*/
#pragma once
#include <algorithm>
#include <iterator>
#include <juce_dsp/juce_dsp.h>
#include "Voicings.h"

namespace gramp
{
class ToneStack
{
public:
    void prepare (double sampleRate, const VoicingRow& row)
    {
        current = row;
        sr = sampleRate;
        gainsValid = false;
        recalc();
        const juce::dsp::ProcessSpec spec { sampleRate, 128, 2 };
        for (auto* f : { &bass, &mid, &treble, &presence, &depth })
            f->prepare (spec);
        reset();
    }

    void reset()
    {
        for (auto* f : { &bass, &mid, &treble, &presence, &depth })
            f->reset();
    }

    void setVoicing (double sampleRate, const VoicingRow& row)
    {
        current = row;
        sr = sampleRate;
        gainsValid = false;
        recalc();
    }

    /*  Knob values 0..1, already smoothed. */
    void setBandGains (float bassK, float midK, float trebleK, float presenceK, float depthK)
    {
        // CR-005: skip the coefficient rebuild (5 heap allocations) when no knob moved
        const float k[5] = { bassK, midK, trebleK, presenceK, depthK };
        if (gainsValid && std::equal (std::begin (k), std::end (k), std::begin (lastK)))
            return;
        std::copy (std::begin (k), std::end (k), std::begin (lastK));
        gainsValid = true;

        const auto mapRange = [] (float knob, float rangeDb)
        {
            return (knob - 0.5f) * rangeDb;
        };

        *bass.state    = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sr, current.bassFreq, 0.9f, juce::Decibels::decibelsToGain (mapRange (bassK, current.bassRangeDb)));
        *mid.state     = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sr, current.midFreq, 0.9f, juce::Decibels::decibelsToGain (mapRange (midK, current.midRangeDb)));
        *treble.state  = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sr, current.trebleFreq, 0.9f, juce::Decibels::decibelsToGain (mapRange (trebleK, current.trebleRangeDb)));
        *presence.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sr, current.presenceFreq, 0.7071f,
            juce::Decibels::decibelsToGain (presenceK * current.presenceRangeDb));
        *depth.state    = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sr, current.depthFreq, 0.7071f,
            juce::Decibels::decibelsToGain (depthK * current.depthRangeDb));
    }

    void processBlock (juce::dsp::AudioBlock<float>& block)
    {
        for (auto* f : { &bass, &mid, &treble, &presence, &depth })
            f->process (juce::dsp::ProcessContextReplacing<float> (block));
    }

private:
    void recalc()   // neutral coefficients until setBandGains is called
    {
        setBandGains (0.5f, 0.5f, 0.5f, 0.0f, 0.0f);
    }

    double sr = 44100.0;
    float lastK[5] = {};
    bool gainsValid = false;
    VoicingRow current = voicings[1];   // Plexi fallback
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> bass, mid, treble, presence, depth;
};

} // namespace gramp
