/*
  TestSupport.h — helpers shared by the frozen unit/dsp tests (T-001..T-013).
*/
#pragma once
#include "../Source/PluginProcessor.h"
#include "../Source/Parameters.h"

#include <random>

namespace gramptest
{
inline juce::File testDataDir()
{
    return juce::File (GRIMAMP_TEST_DATA_DIR);
}

/*  Drives the processor with a callback-driven signal, collecting the output.
    Calls prepareToPlay first; releases at scope end. */
struct RenderContext
{
    GrimAmpAudioProcessor proc;
    double sampleRate;
    int blockSize;

    RenderContext (double rate = 48000.0, int block = 512)
        : sampleRate (rate), blockSize (block)
    {
        proc.prepareToPlay (sampleRate, blockSize);
    }

    juce::AudioBuffer<float> render (int numSamples,
                                     const std::function<void (juce::AudioBuffer<float>&, int blockStart)>& fill)
    {
        juce::AudioBuffer<float> output (2, numSamples);
        juce::AudioBuffer<float> block (2, blockSize);
        juce::MidiBuffer midi;

        for (int start = 0; start < numSamples; start += blockSize)
        {
            const int n = juce::jmin (blockSize, numSamples - start);
            block.clear();
            if (fill != nullptr)
                fill (block, start);
            proc.processBlock (block, midi);
            for (int ch = 0; ch < 2; ++ch)
                output.copyFrom (ch, start, block, ch, 0, n);
        }
        return output;
    }
};

inline float rmsOf (const juce::AudioBuffer<float>& buf, int channel = 0,
                    int start = 0, int num = -1)
{
    if (num < 0)
        num = buf.getNumSamples() - start;
    return buf.getRMSLevel (channel, start, num);
}

inline float peakOf (const juce::AudioBuffer<float>& buf, int channel = 0)
{
    return buf.getMagnitude (channel, 0, buf.getNumSamples());
}

/*  Simple in-memory FFT for harmonic tests (T-006). */
struct MagnitudeSpectrum
{
    explicit MagnitudeSpectrum (const juce::AudioBuffer<float>& buf, int channel,
                                int startSample, int numSamples)
    {
        const int order = juce::roundToInt (std::log2 ((double) numSamples));
        fft = std::make_unique<juce::dsp::FFT> (order);
        size = 1 << order;

        scratch.resize ((size_t) size * 2);
        for (int i = 0; i < size; ++i)
        {
            scratch[(size_t) i * 2]     = (i < numSamples ? buf.getSample (channel, startSample + i) : 0.0f);
            scratch[(size_t) i * 2 + 1] = 0.0f;
        }
        fft->performRealOnlyForwardTransform (scratch.data());
    }

    /*  Magnitude (linear) of the bin nearest to freqHz. */
    float magnitudeAt (double freqHz, double sampleRate) const
    {
        const int bin = juce::roundToInt (freqHz * (double) size / sampleRate);
        return magnitudeAtBin (bin);
    }

    float magnitudeAtBin (int bin) const
    {
        bin = juce::jlimit (1, size / 2 - 1, bin);
        const float re = scratch[(size_t) bin * 2];
        const float im = scratch[(size_t) bin * 2 + 1];
        return std::sqrt (re * re + im * im) / (float) (size / 2);
    }

    std::unique_ptr<juce::dsp::FFT> fft;
    int size = 0;
    std::vector<float> scratch;
};

/*  THD estimate from bins 2..8 of the spectrum of a sine whose fundamental
    sits in bin `fundamentalBin`. */
inline float thd (const MagnitudeSpectrum& spec, int fundamentalBin, int maxHarmonic = 8)
{
    float fund = spec.magnitudeAtBin (fundamentalBin);
    float harm = 0.0f;
    for (int h = 2; h <= maxHarmonic; ++h)
        harm += std::pow (spec.magnitudeAtBin (fundamentalBin * h), 2.0f);
    return fund > 0.0f ? std::sqrt (harm) / fund : 0.0f;
}

/*  Pseudo-random generator for T-009 (seed 1234) — std::mt19937 is
    implementation-stable enough for this use; the seed and engine are part
    of the frozen test. */
inline std::mt19937 makeRng (unsigned seed = 1234u)
{
    return std::mt19937 (seed);
}

inline juce::StringArray readParameterIds()
{
    juce::StringArray ids;
    auto file = testDataDir().getChildFile ("parameter_ids.txt");
    auto lines = juce::StringArray::fromLines (file.loadFileAsString());
    for (auto& line : lines)
    {
        auto t = line.trim();
        if (t.isNotEmpty() && ! t.startsWith ("#"))
            ids.add (t);
    }
    return ids;
}
} // namespace gramptest
