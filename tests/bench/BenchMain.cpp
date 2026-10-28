/*
  BenchMain.cpp — benchmark + render harness (S-013, T-050/T-051, S-009).

  Modes:
    --bench                 T-050 CPU budget render + T-051 latency measurement
                            -> bench-results.json on stdout or --out <file>
    --render-voicings <dir> S-009: render a 10 s DI clip per voicing at defaults
    --render-presets <dir>  S-015 support: render one 10 s clip per factory preset
    --out <file>            JSON output path (default: stdout)
*/
#include "../TestSupport.h"
#include <fstream>
#include <juce_audio_formats/juce_audio_formats.h>

namespace
{
juce::var makeVar (const juce::String& s) { return juce::var (s); }

juce::AudioBuffer<float> loadDiClip (double targetRate, int seconds)
{
    juce::AudioBuffer<float> di (1, (int) (targetRate * seconds));
    auto file = gramptest::testDataDir().getChildFile ("bench").getChildFile ("di.wav");
    juce::AudioFormatManager afm;
    afm.registerFormat (new juce::WavAudioFormat(), true);

    if (auto* reader = afm.createReaderFor (file))
    {
        const int avail = (int) reader->lengthInSamples;
        const int n = juce::jmin (avail, di.getNumSamples());
        reader->read (&di, 0, n, 0, true, false);
        delete reader;
    }
    else
    {
        // fallback synthetic DI if the checked-in file is missing
        for (int i = 0; i < di.getNumSamples(); ++i)
            di.setSample (0, i, 0.25f * (float) std::sin (juce::MathConstants<double>::twoPi * 110.0 * (double) i / targetRate));
    }
    return di;
}

void setParam (GrimAmpAudioProcessor& proc, const char* id, float v)
{
    auto* p = proc.apvts.getParameter (id);
    p->setValueNotifyingHost (p->convertTo0to1 (v));
}

juce::AudioBuffer<float> render (GrimAmpAudioProcessor& proc, const juce::AudioBuffer<float>& di,
                                 int blockSize)
{
    juce::AudioBuffer<float> out (2, di.getNumSamples());
    juce::AudioBuffer<float> block (2, blockSize);
    juce::MidiBuffer midi;
    for (int start = 0; start < di.getNumSamples(); start += blockSize)
    {
        const int n = juce::jmin (blockSize, di.getNumSamples() - start);
        block.clear();
        block.copyFrom (0, 0, di, 0, start, n);
        block.copyFrom (1, 0, di, 0, start, n);
        proc.processBlock (block, midi);
        out.copyFrom (0, start, block, 0, 0, n);
        out.copyFrom (1, start, block, 1, 0, n);
    }
    return out;
}

void writeWav (const juce::File& file, const juce::AudioBuffer<float>& buf, double rate)
{
    file.getParentDirectory().createDirectory();
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> writer (
        wav.createWriterFor (file.createOutputStream().release(), rate, 2, 24, {}, 0));
    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer (buf, 0, buf.getNumSamples());
}

/*  JUCE queues IR loading on a background thread; pump silence until the
    convolution engines actually hold the IR so measurements don't race the
    loader. Bounded at ~4 s of audio. */
void pumpUntilCabReady (GrimAmpAudioProcessor& proc)
{
    juce::AudioBuffer<float> block (2, 256);
    juce::MidiBuffer midi;
    for (int i = 0; i < 750 && ! proc.isCabIrEngineReady(); ++i)
    {
        block.clear();
        proc.processBlock (block, midi);
    }
}

int measureImpulseDelay (GrimAmpAudioProcessor& proc, double rate, int block)
{
    juce::ignoreUnused (rate, block);   // processor is already prepared by the caller
    pumpUntilCabReady (proc);
    // The convolution engine crossfades internally over the IR swap; let that
    // tail settle so the peak position is deterministic (±0-1 sample jitter
    // was observed measuring immediately after readiness).
    {
        juce::AudioBuffer<float> settle (2, block);
        juce::MidiBuffer midi;
        for (int i = 0; i < 64; ++i)
        {
            settle.clear();
            proc.processBlock (settle, midi);
        }
    }
    const int len = 8192;
    // Blocks must not exceed the prepared maximum block size (host contract);
    // stream the impulse through in block-sized chunks and capture the tail.
    juce::AudioBuffer<float> captured (2, len);
    captured.clear();
    juce::AudioBuffer<float> chunk (2, block);
    juce::MidiBuffer midi;
    for (int start = 0; start < len; start += block)
    {
        chunk.clear();
        if (start == 0)
        {
            // Small-signal impulse: latency is a linear-system property. A
            // full-scale impulse drives the plan-mandated waveshapers (D-002/
            // D-003) hard, and the peak of |output| then reports nonlinear
            // energy spread (~4 samples @48k), not processing latency.
            chunk.setSample (0, 0, 0.001f);
            chunk.setSample (1, 0, 0.001f);
        }
        proc.processBlock (chunk, midi);
        const int n = juce::jmin (block, len - start);
        captured.copyFrom (0, start, chunk, 0, 0, n);
        captured.copyFrom (1, start, chunk, 1, 0, n);
    }

    auto peakInfo = [] (const float* d, int n)
    {
        int idx = 0; float best = 0.0f;
        for (int i = 0; i < n; ++i)
            if (std::abs (d[i]) > best) { best = std::abs (d[i]); idx = i; }
        return idx;
    };
    return peakInfo (captured.getReadPointer (0), len);
}

int runBench (const juce::File& outFile)
{
    const double rate = 48000.0;
    const int blockSize = 256;
    const int seconds = 60;

    auto di = loadDiClip (rate, seconds);

    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty ("sampleRate", rate);
    root->setProperty ("blockSize", blockSize);
    root->setProperty ("seconds", seconds);
    root->setProperty ("machine", juce::SystemStats::getCpuModel()
                                       + " / " + juce::String (juce::SystemStats::getNumCpus()) + " cpus");
    auto* voicings = new juce::DynamicObject();
    root->setProperty ("voicings", juce::var (voicings));

    for (int v = 0; v < gramp::numVoicings; ++v)
    {
        for (int os = 0; os < 2; ++os)
        {
            GrimAmpAudioProcessor proc;
            proc.prepareToPlay (rate, blockSize);
            pumpUntilCabReady (proc);
            setParam (proc, gramp::param::ampVoicing, (float) v);
            setParam (proc, gramp::param::oversampling, (float) os);
            setParam (proc, gramp::param::cabIr, (float) gramp::voicings[v].defaultCab);

            // warmup
            render (proc, juce::AudioBuffer<float> (di.getArrayOfWritePointers(), 1, 4096), blockSize);

            const double t0 = juce::Time::getMillisecondCounterHiRes();
            render (proc, di, blockSize);
            const double ms = juce::Time::getMillisecondCounterHiRes() - t0;

            const double audioMs = 1000.0 * (double) di.getNumSamples() / rate;
            const double cpu = 100.0 * ms / audioMs;

            auto* entry = new juce::DynamicObject();
            entry->setProperty ("voicing", gramp::voicings[v].name);
            entry->setProperty ("oversampling", gramp::oversampling::names[os]);
            entry->setProperty ("renderMs", ms);
            entry->setProperty ("cpuPercent", cpu);
            entry->setProperty ("passesBudget15", cpu <= 15.0);
            voicings->setProperty (juce::String (gramp::voicings[v].name) + "/"
                                       + gramp::oversampling::names[os],
                                   juce::var (entry));
        }
    }

    // T-051 latency report
    auto* latency = new juce::DynamicObject();
    root->setProperty ("latency", juce::var (latency));

    for (int os = 0; os < 2; ++os)
    {
        GrimAmpAudioProcessor proc;
        proc.prepareToPlay (rate, 256);
        setParam (proc, gramp::param::oversampling, (float) os);
        setParam (proc, gramp::param::gateOn, 0.0f);   // keep the small-signal
                                                       // impulse out of the gate
        // stepped params apply at the next block boundary (S-012); settle one
        // block so getLatencySamples() reports the selected mode
        {
            juce::AudioBuffer<float> settle (2, 256);
            juce::MidiBuffer midi;
            settle.clear();
            proc.processBlock (settle, midi);
        }
        const int reported = proc.getLatencySamples();
        const int measured = measureImpulseDelay (proc, rate, 256);
        const bool zeroOk = (os == 0 ? reported == 0 : reported <= 144);
        const bool match = std::abs (reported - measured) <= 1;

        auto* entry = new juce::DynamicObject();
        entry->setProperty ("mode", gramp::oversampling::names[os]);
        entry->setProperty ("reported", reported);
        entry->setProperty ("measured", measured);
        entry->setProperty ("passes", zeroOk && match);
        latency->setProperty (gramp::oversampling::names[os], juce::var (entry));
    }

    juce::var json (root);
    auto text = juce::JSON::toString (json, true);

    if (outFile != juce::File())
    {
        outFile.replaceWithText (text);
        std::cout << "wrote " << outFile.getFullPathName() << "\n";
    }
    else
    {
        std::cout << text << "\n";
    }
    return 0;
}







int renderVoicings (const juce::File& dir)
{
    const double rate = 48000.0;
    const int seconds = 10;
    auto di = loadDiClip (rate, seconds);

    for (int v = 0; v < gramp::numVoicings; ++v)
    {
        GrimAmpAudioProcessor proc;
        proc.prepareToPlay (rate, 256);
        setParam (proc, gramp::param::ampVoicing, (float) v);
        setParam (proc, gramp::param::cabIr, (float) gramp::voicings[v].defaultCab);
        auto out = render (proc, di, 256);
        writeWav (dir.getChildFile ("voicing-" + juce::String (gramp::voicings[v].name) + ".wav"),
                  out, rate);
    }
    return 0;
}

int renderPresets (const juce::File& dir)
{
    const double rate = 48000.0;
    const int seconds = 10;
    auto di = loadDiClip (rate, seconds);

    GrimAmpAudioProcessor proc;
    proc.prepareToPlay (rate, 256);
    auto& pm = proc.getPresetManager();

    for (int i = 0; i < pm.getNumPresets(); ++i)
    {
        if (! pm.loadPreset (i))
            continue;
        auto out = render (proc, di, 256);
        auto safe = pm.getPresetInfo (i).name.replaceCharacters (" /", "__");
        writeWav (dir.getChildFile ("preset-" + juce::String (i + 1) + "-" + safe + ".wav"),
                  out, rate);
    }
    return 0;
}
} // namespace

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    juce::StringArray args;
    for (int i = 1; i < argc; ++i)
        args.add (argv[i]);

    juce::File outFile;
    for (int i = 0; i < args.size(); ++i)
        if (args[i] == "--out" && i + 1 < args.size())
            outFile = juce::File (args[++i]);

    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i] == "--bench")
            return runBench (outFile);
        if (args[i] == "--render-voicings" && i + 1 < args.size())
            return renderVoicings (juce::File (args[++i]));
        if (args[i] == "--render-presets" && i + 1 < args.size())
            return renderPresets (juce::File (args[++i]));
    }

    std::cerr << "usage: grimamp_bench --bench [--out f] | --render-voicings <dir> | --render-presets <dir>\n";
    return 2;
}
