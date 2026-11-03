/*
  Preamp.h — two cascaded preamp stages (S-005, D-002/D-003).

  Each stage = pre-emphasis high shelf -> memoryless waveshaper
  (tanh / cubic blend family) -> post-emphasis shelf. All coefficients come
  from the active VoicingRow; the Drive knob scales the per-stage shaper gain.
  The whole preamp runs inside the 4x oversampled wrapper in High mode.
*/
#pragma once
#include <juce_dsp/juce_dsp.h>
#include "BiquadDesign.h"
#include "Voicings.h"

namespace gramp
{
/*  G-010 round 1 (D-001x): the post-shaper shelf undoes only this fraction of the pre-emphasis boost.
    A full undo (1.0) also cut the harmonics the shaper had just made above the shelf corner by the whole
    pre-emphasis gain, which left the 2-8 kHz band 10-25 dB under real-amp references. */
inline constexpr float deEmphasisFraction = 0.5f;

/*  G-010 round 2 (D-001y): the real references keep crest factor ~10 dB and a flat 1-8 kHz spectrum; with
    the whole spectrum going through one memoryless shaper the guitar's low notes dominate the clipping and
    little energy reaches 2-8 kHz (crest 4-7 dB). Real amps high-pass between gain stages (coupling caps).
    The shaper therefore sees the signal above this corner; the lows go through a separate path at this weight. */
inline constexpr float couplingCornerHz = 150.0f;
inline constexpr float lowPathMix = 0.3f;

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
        expSetup (sampleRate);

        preEmphasis.coefficients  = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (preG));
        postEmphasis.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (-preG * deEmphasisFraction));

        const juce::dsp::ProcessSpec spec { sampleRate, 128, 2 };
        preEmphasis.prepare (spec);
        postEmphasis.prepare (spec);
        setDrive (0.5f);
        outGain = juce::Decibels::decibelsToGain (outG);
    }

    /*  Audio-thread safe voicing / rate change: coefficients are redesigned in place and the filter
        memory cleared; no prepare(), no heap allocation (CR-005). prepare() must have run once. */
    void retune (double sampleRate, const VoicingRow& row, bool secondStage)
    {
        const float preF  = secondStage ? row.s2PreFreq  : row.s1PreFreq;
        const float preG  = secondStage ? row.s2PreGainDb : row.s1PreGainDb;
        const float outG  = secondStage ? row.s2OutGainDb : row.s1OutGainDb;
        driveBase         = secondStage ? row.s2Drive     : row.s1Drive;
        tanhBlend         = row.tanhBlend;
        expSetup (sampleRate);

        biquad::highShelf (*preEmphasis.coefficients,  sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (preG));
        biquad::highShelf (*postEmphasis.coefficients, sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (-preG * deEmphasisFraction));
        preEmphasis.reset();
        postEmphasis.reset();
        setDrive (0.5f);
        outGain = juce::Decibels::decibelsToGain (outG);
    }

    void reset()
    {
        hpLp = 0.0f;
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
            // split band: the shaper sees the signal above the coupling corner; the lows take their own,
            // lightly weighted shaper path so they do not mask the highs inside the clipper
            hpLp += hpA * (x - hpLp);
            const float low = hpLp;
            x -= low;
            x = preEmphasis.processSample (x);
            x *= drive;
            x = shape (x);
            x += lowPathMix * shape (low * drive);
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

    void expSetup (double sr)
    {
        hpA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * couplingCornerHz / (float) sr);
        hpLp = 0.0f;
    }
    float hpA = 0.0f, hpLp = 0.0f;
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
        stage1.retune (sampleRate, row, false);
        stage2.retune (sampleRate, row, true);
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
