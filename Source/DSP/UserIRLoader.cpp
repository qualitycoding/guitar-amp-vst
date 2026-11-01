#include "UserIRLoader.h"
#include <cmath>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_events/juce_events.h>

namespace gramp
{
/*  One persistent worker thread. A request only stores the latest file/rate/callback under a
    short lock and wakes the worker (CR-006): the message thread never waits for a load, a newer
    request supersedes an older one (checked between the read, resample and publish steps), and
    shutdown is cooperative. The finished buffer reaches the audio thread through a single atomic
    slot, so take() never takes a mutex. */
class UserIRLoader::Impl : private juce::Thread
{
public:
    Impl() : juce::Thread ("GrimAmp user IR loader") { startThread(); }

    ~Impl() override
    {
        signalThreadShouldExit();
        notify();
        stopThread (10000);
        delete slot.exchange (nullptr);
        delete spent.exchange (nullptr);
    }

    void request (const juce::File& f, double rate, std::function<void (bool)> cb)
    {
        {
            const juce::ScopedLock sl (lock);
            pending = Request { f, rate, std::move (cb), generation.fetch_add (1) + 1 };
            hasPending = true;
            loading = true;
        }
        notify();
    }

    bool take (juce::AudioBuffer<float>& out, double& rateOut)
    {
        Result* r = slot.exchange (nullptr);
        if (r == nullptr)
            return false;

        out = std::move (r->buffer);
        rateOut = r->rate;
        delete spent.exchange (r);   // recycle: the worker frees it, not the audio thread (normally nullptr here)
        return true;
    }

    bool busy() const noexcept { return loading.load() || slot.load() != nullptr; }

private:
    struct Request
    {
        juce::File file;
        double rate = 44100.0;
        std::function<void (bool)> callback;
        int gen = 0;
    };

    struct Result
    {
        juce::AudioBuffer<float> buffer;
        double rate = 44100.0;
    };

    void run() override
    {
        while (! threadShouldExit())
        {
            wait (100);

            for (;;)
            {
                Request req;
                {
                    const juce::ScopedLock sl (lock);
                    if (! hasPending)
                    {
                        loading = false;
                        break;
                    }
                    req = std::move (pending);
                    hasPending = false;
                }

                if (threadShouldExit())
                    return;

                process (req);
            }
        }
    }

    bool superseded (const Request& req) const { return req.gen != generation.load(); }

    void process (const Request& req)
    {
        juce::AudioFormatManager afm;
        afm.registerBasicFormats();

        // Read the whole file (WAV or AIFF). Corrupt/truncated files simply
        // fail here -> previous IR stays active (T-012c).
        std::unique_ptr<juce::AudioFormatReader> reader (afm.createReaderFor (req.file));
        bool ok = false;
        juce::AudioBuffer<float> result;
        const double rate = req.rate;

        if (reader != nullptr && reader->sampleRate > 0.0 && std::isfinite (reader->sampleRate))
        {
            const double srcRate = reader->sampleRate;
            const int maxLen = 2 * 48000 * 30;      // sanity cap: 30 s at 96 kHz, applied to source AND output
            // CR-007: clamp in 64-bit before narrowing, and bound the resampled length, not just the source
            const int len = (int) juce::jmin ((juce::int64) maxLen, reader->lengthInSamples);
            const bool needsResample = std::abs (srcRate - rate) > 1.0;
            const double dstLenD = std::ceil ((double) len * rate / srcRate);
            const bool sizeOk = len > 0 && (! needsResample || (dstLenD >= 1.0 && dstLenD <= (double) maxLen));

            if (sizeOk && ! threadShouldExit() && ! superseded (req))
            {
                juce::AudioBuffer<float> raw (2, len);
                const bool readOk = reader->read (&raw, 0, len, 0, true, true);

                if (readOk && needsResample && ! threadShouldExit() && ! superseded (req))
                {
                    // offline resample to the host rate (AudioFormatReaderSource +
                    // ResamplingAudioSource, as suggested by D-004)
                    juce::AudioFormatReaderSource src (reader.get(), false);
                    src.setNextReadPosition (0);
                    juce::ResamplingAudioSource resampler (&src, false, 2);
                    resampler.setResamplingRatio (srcRate / rate);
                    resampler.prepareToPlay (len, rate);
                    const int dstLen = (int) dstLenD;
                    result.setSize (2, dstLen);
                    juce::AudioSourceChannelInfo info (&result, 0, dstLen);
                    resampler.getNextAudioBlock (info);
                }
                else if (readOk && ! needsResample)
                {
                    result = std::move (raw);
                }

                // reject non-finite content (a NaN/Inf IR would poison the convolution output)
                bool finite = readOk && result.getNumSamples() > 0;
                for (int ch = 0; finite && ch < result.getNumChannels(); ++ch)
                {
                    const float* d = result.getReadPointer (ch);
                    for (int i = 0; i < result.getNumSamples(); ++i)
                        if (! std::isfinite (d[i])) { finite = false; break; }
                }

                ok = finite && ! result.hasBeenCleared();
            }
        }

        if (superseded (req) || threadShouldExit())   // a newer request replaces this one; stay silent
            return;

        if (ok)
        {
            auto* r = new Result { std::move (result), rate };
            delete slot.exchange (r);               // an unread older result is replaced
            delete spent.exchange (nullptr);
        }

        if (req.callback != nullptr)
        {
            auto cb = req.callback;
            juce::MessageManager::callAsync ([cb, ok] { cb (ok); });
        }
    }

    juce::CriticalSection lock;                 // guards `pending` / `hasPending` only, held briefly
    Request pending;
    bool hasPending = false;
    std::atomic<int> generation { 0 };
    std::atomic<bool> loading { false };
    std::atomic<Result*> slot { nullptr };      // finished result, consumed by take()
    std::atomic<Result*> spent { nullptr };     // consumed result awaiting deletion on the worker thread

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
