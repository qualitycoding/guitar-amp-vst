/*
  Preamp.h — two cascaded preamp stages (S-005, D-002/D-003).

  Each stage = pre-emphasis high shelf -> memoryless waveshaper
  (tanh / cubic blend family) -> post-emphasis shelf. All coefficients come
  from the active VoicingRow; the Drive knob scales the per-stage shaper gain.
  The whole preamp runs inside the 4x oversampled wrapper in High mode.
*/
#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Voicings.h"

namespace gramp
{
class PreampStage
{
public:
    void prepare (double sampleRate, const VoicingRow& row, bool secondStage)
    {
        const float preF  = secondStage ? row.s2PreFreq  : row.s1PreFreq;
        const float preG  = secondStage ? row.s2PreGainDb : row.s1PreGainDb;
        const float outG  = secondStage ? row.s2OutGainDb : row.s1OutGainDb;
        driveBase         = secondStage ? row.s2Drive     : row.s1Drive;
        tanhBlend         = row.tanhBlend;

        preEmphasis.coefficients  = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (preG));
        postEmphasis.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (-preG));

        const juce::dsp::ProcessSpec spec { sampleRate, 128, 2 };
        preEmphasis.prepare (spec);
        postEmphasis.prepare (spec);
        setDrive (0.5f);
        outGain = juce::Decibels::decibelsToGain (outG);
    }

    void reset()
    {
        preEmphasis.reset();
        postEmphasis.reset();
    }

    void setDrive (float driveKnob) noexcept   // 0..1, smoothed upstream
    {
        drive = driveBase * (0.25f + 3.75f * juce::jlimit (0.0f, 1.0f, driveKnob));
    }

    void process (float* const data, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];
            x = preEmphasis.processSample (x);
            x *= drive;
            x = shape (x);
            x *= outGain;
            x = postEmphasis.processSample (x);
            data[i] = x;
        }
    }

private:
    float shape (float x) const noexcept
    {
        // cubic branch: x - x^3/3 inside [-1, 1], clamped to +/-2/3 outside
        float cubic = x;
        if (x > 1.0f)       cubic =  2.0f / 3.0f;
        else if (x < -1.0f) cubic = -2.0f / 3.0f;
        else                cubic = x - (x * x * x) / 3.0f;

        const float th = std::tanh (x);
        return (1.0f - tanhBlend) * cubic + tanhBlend * th;
    }

    float driveBase = 1.0f, drive = 1.0f, outGain = 1.0f, tanhBlend = 0.5f;
    juce::dsp::IIR::Filter<float> preEmphasis, postEmphasis;
};

class Preamp
{
public:
    void prepare (double sampleRate, const VoicingRow& row)
    {
        stage1.prepare (sampleRate, row, false);
        stage2.prepare (sampleRate, row, true);
    }

    void reset()
    {
        stage1.reset();
        stage2.reset();
    }

    void setVoicing (double sampleRate, const VoicingRow& row, float driveKnob)
    {
        prepare (sampleRate, row);
        setDrive (driveKnob);
    }

    void setDrive (float driveKnob) noexcept
    {
        stage1.setDrive (driveKnob);
        stage2.setDrive (driveKnob);
    }

    void process (float* const data, int numSamples) noexcept
    {
        stage1.process (data, numSamples);
        stage2.process (data, numSamples);
    }

private:
    PreampStage stage1, stage2;
};

} // namespace gramp
