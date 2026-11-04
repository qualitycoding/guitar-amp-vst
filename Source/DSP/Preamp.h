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

/*  G-010 research round (D-001aa): real preamp stages are not memoryless and not symmetric. Grid current and
    the cathode bypass capacitor shift each stage's operating point with the signal envelope, so clipping is
    asymmetric (even harmonics) and the shift relaxes between notes (Pakarinen & Yeh 2009; Karjalainen et al.
    2006; Kuroki & Ito 1998). Each stage biases its shaper by a fixed part plus an envelope-driven part, and a
    15 Hz coupling high-pass removes the resulting DC. Measured against the real-amp references this restores
    the crest factor (6.9 to 10.4 dB; references 11.1) and the 80-250 Hz weight. Constants are pooled, not per clip. */
inline constexpr float stageBiasStatic = 0.4f;
inline constexpr float stageBiasDynamic = 0.5f;
inline constexpr float stageEnvAttackSec = 0.001f;
inline constexpr float stageEnvReleaseSec = 0.05f;
inline constexpr float stageCouplingHz = 15.0f;

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
        setupBias (sampleRate);

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
        setupBias (sampleRate);

        biquad::highShelf (*preEmphasis.coefficients,  sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (preG));
        biquad::highShelf (*postEmphasis.coefficients, sampleRate, preF, 0.7071f, juce::Decibels::decibelsToGain (-preG * deEmphasisFraction));
        preEmphasis.reset();
        postEmphasis.reset();
        setDrive (0.5f);
        outGain = juce::Decibels::decibelsToGain (outG);
    }

    void reset()
    {
        env = dcLp = 0.0f;
        preEmphasis.reset();
        postEmphasis.reset();
    }

    void setDrive (float driveKnob) noexcept   // 0..1, smoothed upstream
    {
        // D-001z: square-root knob law. A linear law left mid-knob settings with too little gain: the low-drive
        // presets made 15-35 dB less 2-8 kHz energy than real amps (G-010 rounds 1-2). Ends of travel are unchanged.
        drive = driveBase * (0.25f + 3.75f * std::sqrt (juce::jlimit (0.0f, 1.0f, driveKnob)));
    }

    void process (float* const data, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float x = data[i];
            x = preEmphasis.processSample (x);
            x *= drive;
            {
                const float a = std::abs (x);
                env += (a > env ? envAtk : envRel) * (a - env);
                const float b = stageBiasStatic + stageBiasDynamic * env;
                const float y = shape (x + b) - shape (b);    // zero in, zero out: silence stays silent (T-004)
                dcLp += dcA * (y - dcLp);
                x = y - dcLp;
            }
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

    void setupBias (double sr) noexcept
    {
        envAtk = 1.0f - std::exp (-1.0f / (stageEnvAttackSec * (float) sr));
        envRel = 1.0f - std::exp (-1.0f / (stageEnvReleaseSec * (float) sr));
        dcA = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * stageCouplingHz / (float) sr);
        env = dcLp = 0.0f;
    }
    float env = 0.0f, dcLp = 0.0f, envAtk = 0.0f, envRel = 0.0f, dcA = 0.0f;
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
