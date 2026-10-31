#include "UserIRLoader.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_events/juce_events.h>

namespace gramp
{
class UserIRLoader::Impl : private juce::Thread
{
public:
    Impl() : juce::Thread ("GrimAmp user IR loader") {}

    ~Impl() override
    {
        stopThread (2000);
    }

    void request (const juce::File& f, double rate, std::function<void (bool)> cb)
    {
        {
            const juce::ScopedLock sl (lock);
            fileToLoad = f;
            targetRate = rate;
            callback = std::move (cb);
            generation.fetch_add (1);
            loading = true;
        }
        startThread();
    }

    bool take (juce::AudioBuffer<float>& out, double& rateOut)
    {
        if (! ready.exchange (false))
            return false;

        const juce::ScopedLock sl (lock);
        out = std::move (loadedBuffer);
        rateOut = loadedRate;
        return true;
    }

    bool busy() const noexcept { return loading.load() || ready.load(); }

private:
    void run() override
    {
        juce::AudioFormatManager afm;
        afm.registerBasicFormats();

        const juce::ScopedLock sl (lock);
        const auto file = fileToLoad;
        const auto rate = targetRate;
        const auto gen = generation.load();
        auto cb = callback;

        // Read the whole file (WAV or AIFF). Corrupt/truncated files simply
        // fail here -> previous IR stays active (T-012c).
        std::unique_ptr<juce::AudioFormatReader> reader (afm.createReaderFor (file));
        bool ok = false;
        juce::AudioBuffer<float> result;
        double resultRate = rate;

        if (reader != nullptr && reader->sampleRate > 0.0)
        {
            const double srcRate = reader->sampleRate;
            const int srcLen = (int) reader->lengthInSamples;
            const int maxLen = 2 * 48000 * 30;      // sanity cap: 30 s at 96 kHz
            const int len = juce::jmin (srcLen, maxLen);

            if (len > 0)
            {
                juce::AudioBuffer<float> raw (2, len);
                reader->read (&raw, 0, len, 0, true, true);

                if (std::abs (srcRate - rate) > 1.0)
                {
                    // offline resample to the host rate (AudioFormatReaderSource +
                    // ResamplingAudioSource, as suggested by D-004)
                    juce::AudioFormatReaderSource src (reader.get(), false);
                    src.setNextReadPosition (0);
                    juce::ResamplingAudioSource resampler (&src, false, 2);
                    resampler.setResamplingRatio (srcRate / rate);
                    resampler.prepareToPlay (len, rate);
                    const int dstLen = (int) std::ceil ((double) len * rate / srcRate);
                    result.setSize (2, dstLen);
                    juce::AudioSourceChannelInfo info (&result, 0, dstLen);
                    resampler.getNextAudioBlock (info);
                }
                else
                {
                    result = raw;
                }

                resultRate = rate;
                ok = result.getNumSamples() > 0
                     && ! result.hasBeenCleared();
            }
        }

        if (gen == generation.load())   // a newer request superseded this one
        {
            loadedBuffer = std::move (result);
            loadedRate = resultRate;
            ready.store (ok);
            loading.store (false);

            if (cb != nullptr)
                juce::MessageManager::callAsync ([cb, ok] { cb (ok); });
        }
    }

    juce::CriticalSection lock;
    juce::File fileToLoad;
    double targetRate = 44100.0;
    std::function<void (bool)> callback;
    std::atomic<int> generation { 0 };
    std::atomic<bool> loading { false }, ready { false };

    juce::AudioBuffer<float> loadedBuffer;
    double loadedRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE (Impl)
};

UserIRLoader::UserIRLoader() : impl (std::make_unique<Impl>()) {}
UserIRLoader::~UserIRLoader() = default;

void UserIRLoader::loadFileAsync (const juce::File& file, double targetSampleRate,
                                  std::function<void (bool)> callback)
{
    impl->request (file, targetSampleRate, std::move (callback));
}

bool UserIRLoader::takeLoadedBuffer (juce::AudioBuffer<float>& bufferOut, double& rateOut)
{
    return impl->take (bufferOut, rateOut);
}

bool UserIRLoader::isBusy() const noexcept
{
    return impl->busy();
}

} // namespace gramp
