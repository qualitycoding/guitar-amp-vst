/*
  BiquadDesign.h — in-place RBJ biquad design (CR-005).

  juce::dsp::IIR::Coefficients<float>::makePeakFilter/makeLowShelf/makeHighShelf return a freshly
  heap-allocated object, which the processor must not do on the audio thread. These functions write
  the same five normalised coefficients straight into an existing Coefficients object. The formulas
  and the float arithmetic mirror JUCE's (juce_IIRFilter.cpp) so the results agree; tests compare
  them against the JUCE factory functions.

  The first call on a default-constructed object (not yet five coefficients long) falls back to an
  allocating assignment, so call each design once from prepare() before the audio thread uses it.
*/
#pragma once
#include <cmath>
#include <juce_dsp/juce_dsp.h>

namespace gramp::biquad
{
using Coeffs = juce::dsp::IIR::Coefficients<float>;

inline void assign (Coeffs& c, float b0, float b1, float b2, float a0, float a1, float a2)
{
    if (c.coefficients.size() != 5)
    {
        Coeffs::Ptr fresh (new Coeffs (b0, b1, b2, a0, a1, a2));   // initialisation path only
        c = *fresh;
        return;
    }

    const float inv = 1.0f / a0;
    float* r = c.getRawCoefficients();
    r[0] = b0 * inv;
    r[1] = b1 * inv;
    r[2] = b2 * inv;
    r[3] = a1 * inv;
    r[4] = a2 * inv;
}

inline void peak (Coeffs& c, double sampleRate, float frequency, float Q, float gainFactor)
{
    const float A = juce::jmax (0.0f, std::sqrt (gainFactor));
    const float omega = (2.0f * juce::MathConstants<float>::pi * juce::jmax (frequency, 2.0f)) / (float) sampleRate;
    const float alpha = std::sin (omega) / (Q * 2.0f);
    const float c2 = -2.0f * std::cos (omega);
    const float alphaTimesA = alpha * A;
    const float alphaOverA = alpha / A;
    assign (c, 1.0f + alphaTimesA, c2, 1.0f - alphaTimesA, 1.0f + alphaOverA, c2, 1.0f - alphaOverA);
}

inline void lowShelf (Coeffs& c, double sampleRate, float cutOff, float Q, float gainFactor)
{
    const float A = juce::jmax (0.0f, std::sqrt (gainFactor));
    const float aminus1 = A - 1.0f;
    const float aplus1 = A + 1.0f;
    const float omega = (2.0f * juce::MathConstants<float>::pi * juce::jmax (cutOff, 2.0f)) / (float) sampleRate;
    const float coso = std::cos (omega);
    const float beta = std::sin (omega) * std::sqrt (A) / Q;
    const float aminus1TimesCoso = aminus1 * coso;
    assign (c, A * (aplus1 - aminus1TimesCoso + beta),
               A * 2.0f * (aminus1 - aplus1 * coso),
               A * (aplus1 - aminus1TimesCoso - beta),
               aplus1 + aminus1TimesCoso + beta,
               -2.0f * (aminus1 + aplus1 * coso),
               aplus1 + aminus1TimesCoso - beta);
}

inline void highShelf (Coeffs& c, double sampleRate, float cutOff, float Q, float gainFactor)
{
    const float A = juce::jmax (0.0f, std::sqrt (gainFactor));
    const float aminus1 = A - 1.0f;
    const float aplus1 = A + 1.0f;
    const float omega = (2.0f * juce::MathConstants<float>::pi * juce::jmax (cutOff, 2.0f)) / (float) sampleRate;
    const float coso = std::cos (omega);
    const float beta = std::sin (omega) * std::sqrt (A) / Q;
    const float aminus1TimesCoso = aminus1 * coso;
    assign (c, A * (aplus1 + aminus1TimesCoso + beta),
               A * -2.0f * (aminus1 + aplus1 * coso),
               A * (aplus1 + aminus1TimesCoso - beta),
               aplus1 - aminus1TimesCoso + beta,
               2.0f * (aminus1 - aplus1 * coso),
               aplus1 - aminus1TimesCoso - beta);
}

} // namespace gramp::biquad
