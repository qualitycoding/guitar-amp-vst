/*
  OversampledStage.h — 4x polyphase wrapper around the nonlinear amp region
  (S-005, D-003).

  In High mode the wrapped region (preamp -> tone stack -> power amp) runs at
  4x the host rate via dsp::Oversampling (filterHalfBandPolyphaseIIR). In Eco
  mode the region runs at the host rate and the wrapper is a pass-through
  (zero added latency).
*/
#pragma once

#include <juce_dsp/juce_dsp.h>

namespace gramp
{
class OversampledStage
{
public:
    void prepare (const juce::dsp::ProcessSpec& baseSpec, bool highQuality)
    {
        baseRate = baseSpec.sampleRate;
        channels = (int) baseSpec.numChannels;

        // CR-005: the 4x oversampler is always built here (message thread) so that switching between
        // Eco and High at run time (setHighQuality) allocates nothing.
        oversampling = std::make_unique<juce::dsp::Oversampling<float>> (
            baseSpec.numChannels, 4,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            false);
        /*  JUCE 8 Oversampling manages only buffer sizing (no prepare()):
            the base-rate max block size is fixed here; the oversampled rate
            is implied (factor x) for the wrapped processing. */
        oversampling->initProcessing ((size_t) baseSpec.maximumBlockSize);
        highLatency = (int) oversampling->getLatencyInSamples();

        high = highQuality;
        latency = high ? highLatency : 0;
    }

    /*  Audio-thread safe Eco/High switch: no allocation, the oversampler memory is cleared. */
    void setHighQuality (bool highQuality) noexcept
    {
        high = highQuality && oversampling != nullptr;
        latency = high ? highLatency : 0;
        if (oversampling != nullptr)
            oversampling->reset();
    }

    void reset() noexcept
    {
        if (oversampling != nullptr)
            oversampling->reset();
    }

    bool isHighQuality() const noexcept      { return high; }
    int  getLatency() const noexcept         { return latency; }
    double getInternalSampleRate() const     { return high ? baseRate * 4.0 : baseRate; }
    int  getInternalMaxBlockSize (int baseMax) const { return high ? baseMax * 4 : baseMax; }

    /*  Runs fn(dsp::AudioBlock<float>&, double sampleRate) on the internal
        (possibly 4x) buffer, then returns the processed base-rate block.
        fn must process numSamples*factor samples per channel. */
    template <typename Fn>
    void process (juce::dsp::AudioBlock<float>& baseBlock, Fn&& fn)
    {
        if (! high || oversampling == nullptr)
        {
            fn (baseBlock, baseRate);
            return;
        }

        auto osBlock = oversampling->processSamplesUp (baseBlock);
        fn (osBlock, baseRate * 4.0);
        oversampling->processSamplesDown (baseBlock);
    }

private:
    double baseRate = 44100.0;
    int channels = 2;
    bool high = false;
    int latency = 0;
    int highLatency = 0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
};

} // namespace gramp
