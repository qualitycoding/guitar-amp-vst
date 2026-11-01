/*
  UserIRLoader — background-thread WAV/AIFF loading for the user IR slot
  (S-008, D-004).

  The file is read and resampled to the host rate on a worker thread, then
  handed to CabIR on the audio thread (which forwards it to dsp::Convolution
  using its wait-free loadImpulseResponse). On any failure the previous IR
  stays active (T-012c).
*/
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

namespace gramp
{
class CabIR;   // defined in CabIR.h

class UserIRLoader
{
public:
    UserIRLoader();
    ~UserIRLoader();

    /*  Starts loading a file in the background. If a load is already running
        it is abandoned (the new request wins). callback fires on the message
        thread when the load finishes (success or failure). */
    void loadFileAsync (const juce::File& file, double targetSampleRate,
                        std::function<void (bool success)> callback);

    /*  Poll from the audio thread at block boundary: returns a loaded buffer
        if one is ready, resetting the ready flag. */
    bool takeLoadedBuffer (juce::AudioBuffer<float>& bufferOut, double& rateOut);

    bool isBusy() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE (UserIRLoader)
};

} // namespace gramp
