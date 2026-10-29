/*
  MutationTests.cpp — additional (non-frozen) DSP characterisation tests added for T-070.

  Each test drives one DSP stage with a deterministic signal and compares the
  output against an independent re-statement of the stage's documented
  behaviour, so that arithmetic and comparison mutants in the stage are
  detected. The frozen suite (UnitTests.cpp, T-001..T-013) is untouched.
*/
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

#include <cmath>
#include <vector>

#include "DSP/Boost.h"
#include "DSP/NoiseGate.h"
#include "DSP/PowerAmp.h"
#include "DSP/ToneStack.h"
#include "DSP/Voicings.h"

namespace
{
using Filter = juce::dsp::IIR::Filter<float>;
using Coeffs = juce::dsp::IIR::Coefficients<float>;

constexpr double kRate = 48000.0;
constexpr float kTol = 2.0e-4f;

std::vector<float> sine (int n, double freq, float amp)
{
    std::vector<float> v ((size_t) n);
    for (int i = 0; i < n; ++i)
        v[(size_t) i] = amp * (float) std::sin (2.0 * juce::MathConstants<double>::pi * freq * i / kRate);
    return v;
}

// ------------------------------------------------------------ boost
struct BoostMutationTest : juce::UnitTest
{
    BoostMutationTest() : juce::UnitTest ("boost_reference") {}

    void runTest() override
    {
        beginTest ("boost matches the documented HP -> blend(tanh) -> LP chain, per channel");

        for (const float amount : { 0.0f, 0.3f, 1.0f })
        {
            const int n = 512;
            const auto in0 = sine (n, 1500.0, 0.25f);
            const auto in1 = sine (n, 900.0, 0.08f);

            gramp::Boost boost;
            boost.prepare ({ kRate, (juce::uint32) n, 2 });
            boost.setAmount (amount);

            juce::AudioBuffer<float> buf (2, n);
            buf.copyFrom (0, 0, in0.data(), n);
            buf.copyFrom (1, 0, in1.data(), n);
            juce::dsp::AudioBlock<float> block (buf);
            boost.processBlock (block);

            const float drive = 1.0f + 8.0f * amount;
            const float dry = 1.0f - 0.7f * amount;
            const float wet = 0.9f * amount;

            const std::vector<float>* ins[2] = { &in0, &in1 };
            for (int ch = 0; ch < 2; ++ch)
            {
                Filter hp, lp;
                hp.coefficients = Coeffs::makeHighPass (kRate, 720.0, 0.7071f);
                lp.coefficients = Coeffs::makeLowPass (kRate, 4500.0, 0.7071f);
                float maxErr = 0.0f;
                for (int i = 0; i < n; ++i)
                {
                    const float x = hp.processSample ((*ins[ch])[(size_t) i]);
                    const float y = lp.processSample (dry * x + wet * std::tanh (x * drive));
                    maxErr = juce::jmax (maxErr, std::abs (y - buf.getSample (ch, i)));
                }
                expectLessThan (maxErr, kTol);
            }
        }

        beginTest ("boost amount is clamped to 0..1");
        {
            gramp::Boost a, b;
            for (auto* p : { &a, &b })
                p->prepare ({ kRate, 256, 2 });
            a.setAmount (7.0f);
            b.setAmount (1.0f);
            juce::AudioBuffer<float> ba (2, 256), bb (2, 256);
            const auto s = sine (256, 1200.0, 0.4f);
            for (int ch = 0; ch < 2; ++ch)
            {
                ba.copyFrom (ch, 0, s.data(), 256);
                bb.copyFrom (ch, 0, s.data(), 256);
            }
            juce::dsp::AudioBlock<float> xa (ba), xb (bb);
            a.processBlock (xa);
            b.processBlock (xb);
            for (int i = 0; i < 256; ++i)
                expectWithinAbsoluteError (ba.getSample (0, i), bb.getSample (0, i), 1.0e-6f);

            gramp::Boost c, d;
            for (auto* p : { &c, &d })
                p->prepare ({ kRate, 256, 2 });
            c.setAmount (-3.0f);
            d.setAmount (0.0f);
            juce::AudioBuffer<float> bc (2, 256), bd (2, 256);
            for (int ch = 0; ch < 2; ++ch)
            {
                bc.copyFrom (ch, 0, s.data(), 256);
                bd.copyFrom (ch, 0, s.data(), 256);
            }
            juce::dsp::AudioBlock<float> xc (bc), xd (bd);
            c.processBlock (xc);
            d.processBlock (xd);
            for (int i = 0; i < 256; ++i)
                expectWithinAbsoluteError (bc.getSample (0, i), bd.getSample (0, i), 1.0e-6f);
        }
    }
};

// ------------------------------------------------------------ noise gate
// Independent statement of the documented gate (S-004): 5 ms-class RMS box,
// envelope follower 0.30 / 0.02, 6 dB hysteresis, expander below close threshold,
// 10 ms attack / 100 ms release gain smoothing.
struct GateReference
{
    static constexpr int fifoSize = 4096;
    std::vector<float> fifo = std::vector<float> (fifoSize, 0.0f);
    int fifoPos = 0, fill = 0;
    double sumSq = 0.0;
    float envelope = 0.0f, gain = 1.0f;
    bool open = true;

    void run (float* data, int num, float thrDb)
    {
        const float thrClose = thrDb - 6.0f;
        const float atk = std::exp (-1.0f / (float) (kRate * 0.010));
        const float rel = std::exp (-1.0f / (float) (kRate * 0.100));
        for (int i = 0; i < num; ++i)
        {
            const float x = data[i];
            const float sq = x * x;
            sumSq += (double) sq;
            fifo[(size_t) fifoPos] = sq;
            fifoPos = (fifoPos + 1) % fifoSize;
            if (fill < fifoSize)
                ++fill;
            if (fill == fifoSize)
                sumSq -= (double) fifo[(size_t) fifoPos];
            const int n = juce::jmax (1, fill);
            const float rms = std::sqrt ((float) (sumSq / (double) n));
            envelope += (rms > envelope ? 0.30f : 0.02f) * (rms - envelope);
            const float envDb = juce::Decibels::gainToDecibels (envelope, -120.0f);
            if (open) { if (envDb < thrClose) open = false; }
            else      { if (envDb > thrDb)    open = true;  }
            const float thrLin = juce::Decibels::decibelsToGain (thrClose, 1.0e-6f);
            float target = 1.0f;
            if (! open && envelope < thrLin)
                target = juce::jlimit (0.0f, 1.0f, envelope / juce::jmax (1.0e-6f, thrLin));
            const float g = (target < gain) ? atk : rel;
            gain = g * gain + (1.0f - g) * target;
            data[i] *= gain;
        }
    }
};

struct NoiseGateMutationTest : juce::UnitTest
{
    NoiseGateMutationTest() : juce::UnitTest ("gate_reference") {}

    static std::vector<float> burstThenQuiet (float quietAmp)
    {
        // loud 0.3 s, near-silent 0.7 s, loud again 0.3 s
        std::vector<float> v;
        const auto loud = sine ((int) (0.3 * kRate), 220.0, 0.5f);
        const auto quiet = sine ((int) (0.7 * kRate), 330.0, quietAmp);
        v.insert (v.end(), loud.begin(), loud.end());
        v.insert (v.end(), quiet.begin(), quiet.end());
        v.insert (v.end(), loud.begin(), loud.end());
        return v;
    }

    void runTest() override
    {
        beginTest ("gate output matches the reference over open / close / reopen");

        for (const float thr : { -30.0f, -45.0f })
        {
            auto sig = burstThenQuiet (0.002f);   // about -54 dBFS
            auto ref = sig;

            gramp::NoiseGate gate;
            gate.prepare (kRate, 512);
            gate.setSmoothedThreshold (thr);
            // process in odd-sized chunks so block boundaries are exercised
            for (int pos = 0; pos < (int) sig.size();)
            {
                const int len = juce::jmin (333, (int) sig.size() - pos);
                gate.processBlock (sig.data() + pos, len);
                pos += len;
            }

            GateReference r;
            r.run (ref.data(), (int) ref.size(), thr);

            float maxErr = 0.0f;
            for (size_t i = 0; i < sig.size(); ++i)
                maxErr = juce::jmax (maxErr, std::abs (sig[i] - ref[i]));
            expectLessThan (maxErr, kTol);
        }

        beginTest ("gate passes loud signal and attenuates quiet signal below threshold");
        {
            auto sig = burstThenQuiet (0.002f);
            const auto orig = sig;
            gramp::NoiseGate gate;
            gate.prepare (kRate, 512);
            gate.setSmoothedThreshold (-30.0f);
            gate.processBlock (sig.data(), (int) sig.size());

            // end of the first loud burst is still (nearly) unity
            const int loudEnd = (int) (0.3 * kRate);
            float inLoud = 0.0f, outLoud = 0.0f;
            for (int i = loudEnd - 480; i < loudEnd; ++i)
            {
                inLoud = juce::jmax (inLoud, std::abs (orig[(size_t) i]));
                outLoud = juce::jmax (outLoud, std::abs (sig[(size_t) i]));
            }
            expectGreaterThan (outLoud, 0.95f * inLoud);
            // deep into the quiet section the output is far below the input
            float inPeak = 0.0f, outPeak = 0.0f;
            for (int i = (int) (0.8 * kRate); i < (int) (0.95 * kRate); ++i)
            {
                inPeak = juce::jmax (inPeak, std::abs (orig[(size_t) i]));
                outPeak = juce::jmax (outPeak, std::abs (sig[(size_t) i]));
            }
            expectLessThan (outPeak, 0.2f * inPeak);
        }

        beginTest ("gate is inert before prepare");
        {
            gramp::NoiseGate gate;
            std::vector<float> v = sine (64, 440.0, 0.01f);
            const auto o = v;
            gate.processBlock (v.data(), (int) v.size());
            for (size_t i = 0; i < v.size(); ++i)
                expectEquals (v[i], o[i]);
        }
    }
};

// ------------------------------------------------------------ tone stack
struct ToneStackMutationTest : juce::UnitTest
{
    ToneStackMutationTest() : juce::UnitTest ("tonestack_reference") {}

    void runTest() override
    {
        beginTest ("tone stack matches its documented biquad cascade for every voicing");

        for (int v = 0; v < gramp::numVoicings; ++v)
        {
            const auto& row = gramp::voicings[v];
            const float kb = 0.2f, km = 0.8f, kt = 0.65f, kp = 0.4f, kd = 0.7f;

            gramp::ToneStack ts;
            ts.prepare (kRate, row);
            ts.setBandGains (kb, km, kt, kp, kd);

            const int n = 128;
            const auto in0 = sine (n, 150.0, 0.2f);
            const auto in1 = sine (n, 3000.0, 0.2f);
            juce::AudioBuffer<float> buf (2, n);
            buf.copyFrom (0, 0, in0.data(), n);
            buf.copyFrom (1, 0, in1.data(), n);
            juce::dsp::AudioBlock<float> block (buf);
            ts.processBlock (block);

            const auto dbToGain = [] (float db) { return juce::Decibels::decibelsToGain (db); };
            const std::vector<float>* ins[2] = { &in0, &in1 };
            for (int ch = 0; ch < 2; ++ch)
            {
                Filter f[5];
                f[0].coefficients = Coeffs::makePeakFilter (kRate, row.bassFreq, 0.9f, dbToGain ((kb - 0.5f) * row.bassRangeDb));
                f[1].coefficients = Coeffs::makePeakFilter (kRate, row.midFreq, 0.9f, dbToGain ((km - 0.5f) * row.midRangeDb));
                f[2].coefficients = Coeffs::makePeakFilter (kRate, row.trebleFreq, 0.9f, dbToGain ((kt - 0.5f) * row.trebleRangeDb));
                f[3].coefficients = Coeffs::makeHighShelf (kRate, row.presenceFreq, 0.7071f, dbToGain (kp * row.presenceRangeDb));
                f[4].coefficients = Coeffs::makeLowShelf (kRate, row.depthFreq, 0.7071f, dbToGain (kd * row.depthRangeDb));
                float maxErr = 0.0f;
                for (int i = 0; i < n; ++i)
                {
                    float y = (*ins[ch])[(size_t) i];
                    for (auto& flt : f)
                        y = flt.processSample (y);
                    maxErr = juce::jmax (maxErr, std::abs (y - buf.getSample (ch, i)));
                }
                expectLessThan (maxErr, kTol);
            }
        }

        beginTest ("tone stack is neutral until band gains are set");
        {
            gramp::ToneStack ts;
            ts.prepare (kRate, gramp::voicings[0]);
            const int n = 128;
            const auto in = sine (n, 500.0, 0.3f);
            juce::AudioBuffer<float> buf (2, n);
            for (int ch = 0; ch < 2; ++ch)
                buf.copyFrom (ch, 0, in.data(), n);
            juce::dsp::AudioBlock<float> block (buf);
            ts.processBlock (block);
            for (int i = 0; i < n; ++i)
                expectWithinAbsoluteError (buf.getSample (0, i), in[(size_t) i], 1.0e-4f);
        }
    }
};

// ------------------------------------------------------------ power amp
struct PowerAmpMutationTest : juce::UnitTest
{
    PowerAmpMutationTest() : juce::UnitTest ("poweramp_reference") {}

    void runTest() override
    {
        beginTest ("power amp saturator + sag follow the documented transfer");

        for (int v : { 0, 2, gramp::numVoicings - 1 })
        {
            const auto& row = gramp::voicings[v];
            gramp::PowerAmp pa;
            pa.prepare (kRate, row);
            pa.setSag (0.8f);

            const int n = 1500;
            auto in = sine (n, 110.0, 0.9f);
            auto out = in;
            pa.process (out.data(), n);

            const float atk = std::exp (-1.0f / (float) (kRate * 0.020));
            const float rel = std::exp (-1.0f / (float) (kRate * 0.150));
            const float depth = row.sagAmount * 0.8f;
            const float k = juce::jmax (0.1f, row.powerKnee);
            float env = 0.0f, maxErr = 0.0f;
            for (int i = 0; i < n; ++i)
            {
                const float x = in[(size_t) i];
                const float a = std::abs (x);
                const float c = (a > env) ? atk : rel;
                env = c * env + (1.0f - c) * a;
                const float mod = juce::jlimit (-0.2f, 0.2f, depth * (env - 0.35f));
                const float xs = x * (1.0f - mod);
                float y = std::tanh (k * xs) / std::tanh (k);
                y = juce::jlimit (-1.5f, 1.5f, y + row.powerAsym * 0.35f * (xs * xs));
                maxErr = juce::jmax (maxErr, std::abs (y - out[(size_t) i]));
            }
            expectLessThan (maxErr, kTol);
            expectWithinAbsoluteError (pa.getEnvelope(), env, 1.0e-5f);
        }

        beginTest ("power amp sag knob scales modulation and is clamped to 0..1");
        {
            const auto& row = gramp::voicings[1];
            const auto in = sine (2000, 110.0, 0.9f);
            const auto render = [&] (float knob)
            {
                gramp::PowerAmp pa;
                pa.prepare (kRate, row);
                pa.setSag (knob);
                auto o = in;
                pa.process (o.data(), (int) o.size());
                return o;
            };
            const auto lo = render (0.0f), hi = render (1.0f), over = render (5.0f), under = render (-5.0f);
            float diff = 0.0f;
            for (size_t i = 0; i < in.size(); ++i)
            {
                diff = juce::jmax (diff, std::abs (lo[i] - hi[i]));
                expectWithinAbsoluteError (over[i], hi[i], 1.0e-6f);
                expectWithinAbsoluteError (under[i], lo[i], 1.0e-6f);
            }
            expectGreaterThan (diff, 1.0e-3f);
        }

        beginTest ("thump shelf gain follows the envelope");
        {
            const auto& row = gramp::voicings[3];
            gramp::PowerAmp pa;
            pa.prepare (kRate, row);
            pa.setSag (1.0f);
            const int n = 128;
            auto in = sine (n, 80.0, 0.9f);
            // warm up the envelope
            for (int rep = 0; rep < 20; ++rep)
            {
                auto tmp = in;
                pa.process (tmp.data(), n);
            }
            auto ch0 = in;
            pa.process (ch0.data(), n);
            const float env = pa.getEnvelope();

            juce::AudioBuffer<float> buf (2, n);
            for (int ch = 0; ch < 2; ++ch)
                buf.copyFrom (ch, 0, ch0.data(), n);
            juce::dsp::AudioBlock<float> block (buf);

            // reference filter state is unknown after the warm-up (state is private),
            // so compare against a bounded-gain expectation instead of exact samples:
            // the shelf only boosts lows by 0..6 dB.
            const float before = [&] { float e = 0; for (float s : ch0) e += s * s; return e; }();
            pa.processThump (block, 1.0f);
            float after = 0.0f;
            for (int i = 0; i < n; ++i)
                after += buf.getSample (0, i) * buf.getSample (0, i);
            const float gainDb = juce::jlimit (0.0f, 6.0f, row.sagAmount * env * 8.0f);
            expectGreaterThan (env, 0.1f);
            expectGreaterThan (gainDb, 0.0f);
            expectGreaterThan (after, before * 0.8f);
            expectLessThan (after, before * juce::Decibels::decibelsToGain (2.0f * 6.0f));
        }
    }
};

BoostMutationTest tm1;
NoiseGateMutationTest tm2;
ToneStackMutationTest tm3;
PowerAmpMutationTest tm4;
} // namespace
