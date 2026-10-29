/*
  NoiseGate.h — downward expander with hysteresis (S-004).

  RMS detector (5 ms window), 10 ms attack / 100 ms release on the gain,
  6 dB hysteresis between the open and close thresholds. The threshold itself
  is smoothed per D-006 (setSmoothedThreshold each block).
*/
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace gramp
{
class NoiseGate
{
public:
    void prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        reset();
        juce::ignoreUnused (maxBlockSize);
    }

    void reset()
    {
        runningSumOfSquares = 0.0;
        rmsWindowFill = 0;
        envelope = 0.0;
        gain = 1.0;
        gateOpen = true;
        smoothedThresholdDb = -60.0f;
    }

    void setSmoothedThreshold (float thresholdDb)   // call once per block (D-006)
    {
        smoothedThresholdDb = thresholdDb;
    }

    void processBlock (float* const data, int numSamples) noexcept
    {
        if (sampleRate <= 0.0)
            return;

        const double winLen = juce::jmax (1.0, sampleRate * 0.005);   // 5 ms RMS window
        const float thrOpenDb  = smoothedThresholdDb;
        const float thrCloseDb = smoothedThresholdDb - 6.0f;          // hysteresis (S-004)

        // gain smoothing coefficients: 10 ms attack / 100 ms release
        const float atk = std::exp (-1.0f / (float) (sampleRate * 0.010));
        const float rel = std::exp (-1.0f / (float) (sampleRate * 0.100));

        for (int i = 0; i < numSamples; ++i)
        {
            const float x = data[i];

            // running RMS over a ~5 ms box (cheap moving average of squares)
            const float sq = x * x;
            runningSumOfSquares += (double) sq;
            fifo[(size_t) fifoPos] = sq;
            fifoPos = (fifoPos + 1) % fifoSize;
            if (rmsWindowFill < fifoSize)
                ++rmsWindowFill;
            if (rmsWindowFill == fifoSize)
                runningSumOfSquares -= (double) fifo[(size_t) fifoPos];

            const int n = juce::jmax (1, rmsWindowFill);
            const float rms = std::sqrt ((float) (runningSumOfSquares / (double) n));

            // envelope follower (fast attack, slow release) on the RMS level
            const float coef = (rms > envelope) ? 0.30f : 0.02f;
            envelope += coef * (rms - envelope);

            // hysteresis state machine
            const float envDb = juce::Decibels::gainToDecibels (envelope, -120.0f);
            if (gateOpen)
            {
                if (envDb < thrCloseDb)
                    gateOpen = false;
            }
            else
            {
                if (envDb > thrOpenDb)
                    gateOpen = true;
            }

            // downward-expander target: unity above threshold, deep cut below
            const float thrLinear = juce::Decibels::decibelsToGain (thrCloseDb);
            float target = 1.0f;
            if (! gateOpen && envelope < thrLinear)
                target = juce::jlimit (0.0f, 1.0f, envelope / juce::jmax (1.0e-6f, thrLinear));

            const float g = (target < gain) ? atk : rel;
            gain = g * gain + (1.0f - g) * target;

            data[i] = x * gain;
        }
    }

private:
    double sampleRate = 0.0;
    static constexpr int fifoSize = 4096;   // >= 5 ms at 768 kHz (4x of 192 kHz)
    float fifo[fifoSize] = {};
    int fifoPos = 0;
    int rmsWindowFill = 0;
    double runningSumOfSquares = 0.0;
    float envelope = 0.0f;
    float gain = 1.0f;
    bool gateOpen = true;
    float smoothedThresholdDb = -60.0f;
};

} // namespace gramp
