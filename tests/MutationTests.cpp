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

#include <juce_audio_formats/juce_audio_formats.h>

#include "TestSupport.h"
#include "DSP/Boost.h"
#include "DSP/CabIR.h"
#include "DSP/UserIRLoader.h"
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
            const float thrLin = juce::Decibels::decibelsToGain (thrClose);
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

        beginTest ("thump shelf gain follows the envelope (exact)");
        {
            const auto& row = gramp::voicings[3];
            gramp::PowerAmp pa;
            pa.prepare (kRate, row);
            pa.setSag (1.0f);

            auto drive = sine (3000, 80.0, 0.9f);
            pa.process (drive.data(), (int) drive.size());
            const float env = pa.getEnvelope();
            const float gainDb = juce::jlimit (0.0f, 6.0f, row.sagAmount * 1.0f * env * 8.0f);
            expectGreaterThan (gainDb, 0.05f);

            const int n = 128;
            const auto in = sine (n, 80.0, 0.5f);
            juce::AudioBuffer<float> buf (2, n);
            for (int ch = 0; ch < 2; ++ch)
                buf.copyFrom (ch, 0, in.data(), n);
            juce::dsp::AudioBlock<float> block (buf);
            pa.processThump (block, 1.0f);

            Filter ref;
            ref.coefficients = Coeffs::makeLowShelf (kRate, 110.0, 0.7071f, juce::Decibels::decibelsToGain (gainDb));
            float maxErr = 0.0f;
            for (int i = 0; i < n; ++i)
                maxErr = juce::jmax (maxErr, std::abs (ref.processSample (in[(size_t) i]) - buf.getSample (0, i)));
            expectLessThan (maxErr, kTol);
        }
    }
};


// ------------------------------------------------------------ cab IR
struct CabIRMutationTest : juce::UnitTest
{
    CabIRMutationTest() : juce::UnitTest ("cabir_selection") {}

    static void pumpUntilReady (gramp::CabIR& cab, int maxBlocks = 2000)
    {
        juce::AudioBuffer<float> scratch (2, 256);
        for (int i = 0; i < maxBlocks && ! cab.isEngineReady(); ++i)
        {
            cab.applyPending();
            juce::AudioBuffer<float> b (2, 256);
            b.clear();
            juce::dsp::AudioBlock<float> blk (b);
            cab.process (blk, scratch);
            juce::Thread::sleep (2);
        }
    }

    static juce::AudioBuffer<float> deltaIr (float value)
    {
        juce::AudioBuffer<float> ir (1, 1);
        ir.setSample (0, 0, value);
        return ir;
    }

    // renders a stereo impulse (ch0 impulse 1.0, ch1 impulse 0.5) through the cab
    static juce::AudioBuffer<float> renderImpulse (gramp::CabIR& cab)
    {
        juce::AudioBuffer<float> scratch (2, 256), b (2, 256);
        b.clear();
        b.setSample (0, 0, 1.0f);
        b.setSample (1, 0, 0.5f);
        juce::dsp::AudioBlock<float> blk (b);
        cab.process (blk, scratch);
        return b;
    }

    void runTest() override
    {
        using gramp::cab::factoryCount;
        using gramp::cab::userSlotIndex;
        const juce::dsp::ProcessSpec spec { kRate, 256, 2 };

        beginTest ("engine readiness is false until an IR is applied");
        {
            gramp::CabIR cab;
            cab.prepare (spec);
            expectEquals (cab.getCurrentCab(), -2);
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), 0);   // first prepare loads the default cab
            pumpUntilReady (cab);
            expect (cab.isEngineReady());
            expectGreaterThan (cab.getEngineIrSize(), 0);
        }

        beginTest ("factory selection is applied, clamped and survives re-prepare");
        {
            gramp::CabIR cab;
            cab.prepare (spec);
            cab.applyPending();
            cab.selectCab (3);
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), 3);
            cab.selectCab (1000);
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), factoryCount - 1);
            cab.selectCab (-5);
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), 0);
            cab.selectCab (2);
            cab.applyPending();
            cab.prepare (spec);
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), 2);
        }

        beginTest ("user slot without a user IR keeps the last factory cab");
        {
            gramp::CabIR cab;
            cab.prepare (spec);
            cab.applyPending();
            cab.selectCab (4);
            cab.applyPending();
            cab.selectCab (userSlotIndex);
            expect (! cab.isUserPending());
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), 4);
            expect (! cab.hasUserIr());
        }

        beginTest ("user IR becomes current, re-prepare re-queues it, factory select cancels it");
        {
            gramp::CabIR cab;
            cab.prepare (spec);
            cab.applyPending();
            cab.setUserImpulseResponse (deltaIr (0.5f), kRate);
            expect (cab.hasUserIr());
            expect (cab.isUserPending());
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), -1);
            expect (! cab.isUserPending());

            cab.prepare (spec);                      // rate change path
            expect (cab.isUserPending());
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), -1);

            cab.selectCab (5);
            expect (! cab.isUserPending());
            cab.applyPending();
            expectEquals (cab.getCurrentCab(), 5);

            cab.selectCab (userSlotIndex);           // valid user IR now exists
            expect (cab.isUserPending());
        }

        beginTest ("mic position and mix shape the wet/dry output (user IR 0.5 on both engines)");
        {
            const auto run = [&] (float mic, float mix)
            {
                gramp::CabIR cab;
                cab.prepare (spec);
                cab.applyPending();
                cab.setUserImpulseResponse (deltaIr (0.5f), kRate);
                cab.applyPending();
                pumpUntilReady (cab);
                cab.setMicPosition (mic);
                cab.setMix (mix);
                return renderImpulse (cab);
            };

            // JUCE scales the loaded IR by an internal factor, so calibrate against the
            // mic = 0, mix = 1 render (only the close engine contributes, gain g0 per unit input)
            const float g0 = run (0.0f, 1.0f).getSample (0, 0);
            expectGreaterThan (g0, 0.1f);
            const float s2 = std::sqrt (0.5f);
            const float halfPi = juce::MathConstants<float>::halfPi;
            const float tol = 2.0e-3f * g0;

            // mix = 1: wet only, g0 * (cos p + sin p)
            expectWithinAbsoluteError (run (0.5f, 1.0f).getSample (0, 0), g0 * 2.0f * s2, tol);
            expectWithinAbsoluteError (run (1.0f, 1.0f).getSample (0, 0), g0, tol);
            expectWithinAbsoluteError (run (0.25f, 1.0f).getSample (0, 0),
                                       g0 * (std::cos (0.25f * halfPi) + std::sin (0.25f * halfPi)), tol);
            // channel 1 carries half the input, so half the output
            expectWithinAbsoluteError (run (0.5f, 1.0f).getSample (1, 0), 0.5f * g0 * 2.0f * s2, tol);
            // mic position clamps
            expectWithinAbsoluteError (run (9.0f, 1.0f).getSample (0, 0), g0, tol);
            expectWithinAbsoluteError (run (-9.0f, 1.0f).getSample (0, 0), g0, tol);

            // mix = 0: bit-exact dry
            const auto dry = run (0.5f, 0.0f);
            expectEquals (dry.getSample (0, 0), 1.0f);
            expectEquals (dry.getSample (1, 0), 0.5f);

            // mix = 0.4, mic 0: 0.6 * dry + 0.4 * g0 * dry, per channel
            const auto mixed = run (0.0f, 0.4f);
            expectWithinAbsoluteError (mixed.getSample (0, 0), 0.6f + 0.4f * g0, tol);
            expectWithinAbsoluteError (mixed.getSample (1, 0), 0.6f * 0.5f + 0.4f * g0 * 0.5f, tol);
            // mix clamps
            expectWithinAbsoluteError (run (0.0f, 7.0f).getSample (0, 0), g0, tol);
            expectEquals (run (0.0f, -7.0f).getSample (0, 0), 1.0f);
        }
    }
};

// ------------------------------------------------------------ user IR loader
struct UserIrLoaderMutationTest : juce::UnitTest
{
    UserIrLoaderMutationTest() : juce::UnitTest ("userir_loader") {}

    static juce::File writeWav (const juce::String& name, double rate, int length, bool ramp = true)
    {
        auto file = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile (name);
        file.deleteFile();
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> w (
            wav.createWriterFor (file.createOutputStream().release(), rate, 1, 16, {}, 0));
        juce::AudioBuffer<float> b (1, length);
        for (int i = 0; i < length; ++i)
            b.setSample (0, i, ramp ? 0.5f * (float) i / (float) length : 0.25f);
        w->writeFromAudioSampleBuffer (b, 0, length);
        return file;
    }

    static bool waitForBuffer (gramp::UserIRLoader& l, juce::AudioBuffer<float>& out, double& rate)
    {
        for (int i = 0; i < 1500; ++i)
        {
            if (l.takeLoadedBuffer (out, rate))
                return true;
            juce::Thread::sleep (4);
        }
        return false;
    }

    void runTest() override
    {
        beginTest ("same-rate file is passed through untouched");
        {
            const auto f = writeWav ("grimamp_mut_same.wav", 48000.0, 1000);
            gramp::UserIRLoader l;
            l.loadFileAsync (f, 48000.0, nullptr);
            juce::AudioBuffer<float> out;
            double rate = 0.0;
            expect (waitForBuffer (l, out, rate));
            expectEquals (rate, 48000.0);
            expectEquals (out.getNumSamples(), 1000);
            expectWithinAbsoluteError (out.getSample (0, 500), 0.25f, 1.0e-3f);
            expectWithinAbsoluteError (out.getSample (1, 500), 0.25f, 1.0e-3f);   // mono is mirrored
            expectWithinAbsoluteError (out.getSample (0, 999), 0.5f * 0.999f, 1.0e-3f);
            f.deleteFile();
        }

        beginTest ("a rate difference of exactly 1 Hz is not resampled");
        {
            const auto f = writeWav ("grimamp_mut_1hz.wav", 48001.0, 1000);
            gramp::UserIRLoader l;
            l.loadFileAsync (f, 48000.0, nullptr);
            juce::AudioBuffer<float> out;
            double rate = 0.0;
            expect (waitForBuffer (l, out, rate));
            expectEquals (out.getNumSamples(), 1000);
            expectWithinAbsoluteError (out.getSample (0, 500), 0.25f, 1.0e-3f);
            f.deleteFile();
        }

        beginTest ("a half-rate file is resampled to twice the length");
        {
            const auto f = writeWav ("grimamp_mut_half.wav", 24000.0, 1000);
            gramp::UserIRLoader l;
            l.loadFileAsync (f, 48000.0, nullptr);
            juce::AudioBuffer<float> out;
            double rate = 0.0;
            expect (waitForBuffer (l, out, rate));
            expectEquals (rate, 48000.0);
            expectEquals (out.getNumSamples(), 2000);
            f.deleteFile();
        }

        beginTest ("a missing file never produces a buffer");
        {
            gramp::UserIRLoader l;
            l.loadFileAsync (juce::File::getSpecialLocation (juce::File::tempDirectory)
                                 .getChildFile ("grimamp_mut_does_not_exist.wav"),
                             48000.0, nullptr);
            juce::AudioBuffer<float> out;
            double rate = 0.0;
            juce::Thread::sleep (300);
            expect (! l.takeLoadedBuffer (out, rate));
        }
    }
};

// ------------------------------------------------------------ preset list
struct PresetListMutationTest : juce::UnitTest
{
    PresetListMutationTest() : juce::UnitTest ("preset_list") {}

    void runTest() override
    {
        beginTest ("factory list has exactly 24 named entries; out-of-range lookups are empty");
        GrimAmpAudioProcessor proc;
        auto& pm = proc.getPresetManager();

        int factory = 0, lastFactoryIndex = -1;
        for (int i = 0; i < pm.getNumPresets(); ++i)
        {
            const auto info = pm.getPresetInfo (i);
            if (info.isFactory)
            {
                ++factory;
                lastFactoryIndex = info.factoryIndex;
                expect (info.name.isNotEmpty());
            }
        }
        expectEquals (factory, 24);
        expectEquals (lastFactoryIndex, 23);

        const auto past = pm.getPresetInfo (pm.getNumPresets());
        expect (past.name.isEmpty());
        expect (past.factoryIndex == -1);
        expect (pm.getPresetInfo (-1).name.isEmpty());
        expect (pm.getPresetInfo (0).name.isNotEmpty());
        expect (pm.getPresetInfo (pm.getNumPresets() - 1).name.isNotEmpty());
    }
};

BoostMutationTest tm1;
NoiseGateMutationTest tm2;
ToneStackMutationTest tm3;
PowerAmpMutationTest tm4;
CabIRMutationTest tm5;
UserIrLoaderMutationTest tm6;
PresetListMutationTest tm7;
} // namespace
