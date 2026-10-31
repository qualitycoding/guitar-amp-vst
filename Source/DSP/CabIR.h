/*
  CabIR.h — cabinet IR convolution (S-008, D-004).

  Two zero-latency dsp::Convolution engines hold the close-mic and off-axis
  variants of the selected cabinet; mic position is an equal-power crossfade
  between them, ir-mix is a dry/wet blend. Factory IRs load from BinaryData
  (the memory loader resamples to the prepared rate as needed); the user slot
  receives buffers from UserIRLoader on the audio thread via a lock-free handoff.

  dsp::Convolution's default construction is zero-latency (Latency{0}), which
  keeps Eco-mode plugin latency at exactly 0 samples (T-051).
*/
#pragma once
#include <juce_dsp/juce_dsp.h>
#include "UserIRLoader.h"
#include "BinaryData.h"

namespace gramp
{
class CabIR
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        closeScratch.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
        offScratch.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);

        closeConv.prepare (spec);
        offConv.prepare (spec);
        prepared = true;

        // force a reload of whatever cab is selected so the new rate takes effect
        if (currentFactory == -1 && lastUserValid)
            hasPendingUser = true;
        else if (currentFactory == -2)
            pendingFactory = 0;             // first prepare: load the default cab
        else if (currentFactory >= 0)
            pendingFactory = currentFactory;
    }

    void reset()
    {
        closeConv.reset();
        offConv.reset();
    }

    /*  Select a factory cab (0..7) or the user slot (cab::userSlotIndex). */
    void selectCab (int cabIndex)
    {
        if (cabIndex == cab::userSlotIndex)
        {
            if (lastUserValid)
                hasPendingUser = true;
            else if (currentFactory >= 0)
                pendingFactory = currentFactory;    // no user IR yet: keep last factory
        }
        else
        {
            pendingFactory = juce::jlimit (0, cab::factoryCount - 1, cabIndex);
            hasPendingUser = false;
        }
    }

    /*  Called from the audio thread at block boundary: applies any pending
        factory/user IR swap (loadImpulseResponse is wait-free, see JUCE docs). */
    void applyPending()
    {
        if (! prepared)
            return;

        if (hasPendingUser)
        {
            // each engine takes ownership of its own copy
            juce::AudioBuffer<float> first (userIrBuffer), second (userIrBuffer);
            closeConv.loadImpulseResponse (std::move (first),  userIrRate,
                                           juce::dsp::Convolution::Stereo::no,
                                           juce::dsp::Convolution::Trim::no,
                                           juce::dsp::Convolution::Normalise::no);
            offConv.loadImpulseResponse (std::move (second), userIrRate,
                                         juce::dsp::Convolution::Stereo::no,
                                         juce::dsp::Convolution::Trim::no,
                                         juce::dsp::Convolution::Normalise::no);
            currentFactory = -1;
            hasPendingUser = false;
            pendingFactory = -1;   // cancel any stale factory fallback (e.g. selected
                                   // user slot before the IR decoded): it would
                                   // otherwise reload the old cab over the user IR
                                   // on the next block
            return;
        }

        if (pendingFactory >= 0 && pendingFactory != currentFactory)
        {
            for (int engine = 0; engine < 2; ++engine)
            {
                const auto name = cab::getBinaryDataName (pendingFactory, engine == 1);
                const void* data = nullptr;
                int size = 0;

                getNamedBinaryData (name, data, size);

                if (data != nullptr && size > 0)
                {
                    auto& conv = (engine == 0 ? closeConv : offConv);
                    // Trim::yes strips the IR's leading dead air so the cab adds
                    // no unreported onset delay (T-051: measured impulse delay
                    // must match getLatencySamples() within ±1 sample in Eco).
                    conv.loadImpulseResponse (data, (size_t) size,
                                              juce::dsp::Convolution::Stereo::no,
                                              juce::dsp::Convolution::Trim::yes,
                                              0,
                                              juce::dsp::Convolution::Normalise::no);
                }
            }
            currentFactory = pendingFactory;
        }
    }

    /*  Audio-thread handoff from the loader thread (takes ownership). */
    void setUserImpulseResponse (juce::AudioBuffer<float>&& buffer, double rate)
    {
        userIrBuffer = std::move (buffer);
        lastUserValid = true;
        userIrRate = rate;
        hasPendingUser = true;      // becomes audible as soon as the audio thread applies it
    }

    bool hasUserIr() const noexcept         { return lastUserValid; }
    int  getCurrentCab() const noexcept     { return currentFactory; }

    /*  True once the convolution engines have a live impulse response.
        JUCE loads IRs on a background thread (loadImpulseResponse is merely
        queued), so right after prepare/selectCab the engines can still be
        empty and the cab momentarily inaudible; poll this from a warmup
        loop before measuring (T-050/T-051). */
    bool isEngineReady() const noexcept
    {
        return closeConv.getCurrentIRSize() > 0 && offConv.getCurrentIRSize() > 0;
    }

    int getEngineIrSize() const noexcept   // 0 while the background engine holds nothing
    {
        return closeConv.getCurrentIRSize();
    }

    bool isUserPending() const noexcept { return hasPendingUser; }

    void setMicPosition (float position01) noexcept
    {
        const float p = juce::jlimit (0.0f, 1.0f, position01) * juce::MathConstants<float>::halfPi;
        gainClose = std::cos (p);
        gainOff   = std::sin (p);
    }

    void setMix (float mix01) noexcept { mix = juce::jlimit (0.0f, 1.0f, mix01); }

    /*  block is in-place (input == output). scratchDry must hold at least
        block.getNumSamples() samples per channel and is used for the dry path. */
    void process (juce::dsp::AudioBlock<float>& block, juce::AudioBuffer<float>& scratchDry)
    {
        const float m = mix;
        if (m <= 0.0f)
            return;

        const size_t numChannels = block.getNumChannels();
        const size_t numSamples  = block.getNumSamples();

        scratchDry.setSize ((int) numChannels, (int) numSamples, false, false, true);
        for (size_t ch = 0; ch < numChannels; ++ch)
            scratchDry.copyFrom ((int) ch, 0, block.getChannelPointer (ch), (int) numSamples);

        auto closeBlock = juce::dsp::AudioBlock<float> (closeScratch).getSubBlock (0, numSamples);
        auto offBlock   = juce::dsp::AudioBlock<float> (offScratch).getSubBlock (0, numSamples);

        copyBlock (block, closeBlock);
        copyBlock (block, offBlock);

        closeConv.process (juce::dsp::ProcessContextReplacing<float> (closeBlock));
        offConv.process   (juce::dsp::ProcessContextReplacing<float> (offBlock));

        const float gC = gainClose, gO = gainOff;
        const float wet = m, dry = 1.0f - m;

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* out = block.getChannelPointer (ch);
            auto* d   = scratchDry.getReadPointer ((int) ch);
            auto* c   = closeBlock.getChannelPointer (ch);
            auto* o   = offBlock.getChannelPointer (ch);

            for (size_t i = 0; i < numSamples; ++i)
                out[i] = dry * d[i] + wet * (gC * c[i] + gO * o[i]);
        }
    }

private:
    static void copyBlock (const juce::dsp::AudioBlock<float>& src,
                           juce::dsp::AudioBlock<float>& dst)
    {
        for (size_t ch = 0; ch < src.getNumChannels(); ++ch)
            juce::FloatVectorOperations::copy (dst.getChannelPointer (ch),
                                               src.getChannelPointer (ch),
                                               (int) src.getNumSamples());
    }

    static void getNamedBinaryData (const char* name, const void*& data, int& size)
    {
        data = nullptr;
        size = 0;
        // BinaryData symbols generated by juce_add_binary_data; dispatch by name.
        #define GRIMAMP_IR_ENTRY(symbol) \
            if (juce::String (name) == #symbol) { data = BinaryData::symbol; size = BinaryData::symbol##Size; return; }

        GRIMAMP_IR_ENTRY (cab1_close_wav) GRIMAMP_IR_ENTRY (cab1_off_wav)
        GRIMAMP_IR_ENTRY (cab2_close_wav) GRIMAMP_IR_ENTRY (cab2_off_wav)
        GRIMAMP_IR_ENTRY (cab3_close_wav) GRIMAMP_IR_ENTRY (cab3_off_wav)
        GRIMAMP_IR_ENTRY (cab4_close_wav) GRIMAMP_IR_ENTRY (cab4_off_wav)
        GRIMAMP_IR_ENTRY (cab5_close_wav) GRIMAMP_IR_ENTRY (cab5_off_wav)
        GRIMAMP_IR_ENTRY (cab6_close_wav) GRIMAMP_IR_ENTRY (cab6_off_wav)
        GRIMAMP_IR_ENTRY (cab7_close_wav) GRIMAMP_IR_ENTRY (cab7_off_wav)
        GRIMAMP_IR_ENTRY (cab8_close_wav) GRIMAMP_IR_ENTRY (cab8_off_wav)
        #undef GRIMAMP_IR_ENTRY
    }

    juce::dsp::Convolution closeConv, offConv;

    juce::AudioBuffer<float> userIrBuffer;
    double userIrRate = 44100.0;
    bool hasPendingUser = false, lastUserValid = false;

    int pendingFactory = 0, currentFactory = -2;   // -2: nothing loaded yet
    bool prepared = false;

    float gainClose = 0.7071f, gainOff = 0.7071f, mix = 1.0f;

    juce::AudioBuffer<float> closeScratch { 2, 8192 };
    juce::AudioBuffer<float> offScratch   { 2, 8192 };
};

} // namespace gramp
