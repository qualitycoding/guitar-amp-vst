/*
  GrimAmpAudioProcessor — rock & metal guitar amp (D-002 chain).

  Input Gain -> Noise Gate -> Boost (TS) -> [Preamp 1 -> Preamp 2 -> Tone Stack
  -> Power Amp] @4x (High mode) -> Cab IR (mic crossfade, dry/wet) -> Post EQ
  -> Master. All continuous parameters smoothed (D-006); stepped parameters
  (voicing / cab / oversampling) switch at block boundaries with a 5 ms
  crossfade (S-012). Offline only: JUCE_USE_CURL=0, no network (D-010).
*/
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "Parameters.h"
#include "DSP/NoiseGate.h"
#include "DSP/Boost.h"
#include "DSP/Preamp.h"
#include "DSP/ToneStack.h"
#include "DSP/PowerAmp.h"
#include "DSP/OversampledStage.h"
#include "DSP/CabIR.h"
#include "DSP/PostEq.h"
#include "Presets/PresetManager.h"

class GrimAmpAudioProcessor : public juce::AudioProcessor
{
public:
    GrimAmpAudioProcessor();
    ~GrimAmpAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    bool isMidiEffect() const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // exposed for tests / GUI
    gramp::PresetManager& getPresetManager() { return presetManager; }
    int getCurrentVoicingIndex() const noexcept { return currentVoicing; }
    void requestUserIrLoad (const juce::File& file);
    bool isUserIrLoading() const noexcept;
    bool isUserIrApplied() const noexcept { return userIrApplied; }

    /*  True once the cabinet convolution engines hold a live IR (JUCE loads
        IRs on a background thread after prepare/selectCab). */
    bool isCabIrEngineReady() const { return cabIR.isEngineReady(); }
    int  getCabIrEngineSize() const { return cabIR.getEngineIrSize(); }
    bool isCabIrUserPending() const { return cabIR.isUserPending(); }
    int  getCurrentCabIndex() const { return cabIR.getCurrentCab(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    void pullSmoothParams (int numSamples);
    void applySteppedParams();
    void updateLatency();
    juce::AudioBuffer<float>& getScratch (int numChannels, int numSamples);

    gramp::NoiseGate gate;
    gramp::Boost boost;
    gramp::Preamp preamp;
    gramp::ToneStack toneStack;
    gramp::PowerAmp powerAmp;
    gramp::OversampledStage osStage;
    gramp::CabIR cabIR;
    gramp::PostEq postEq;
    gramp::UserIRLoader userIrLoader;
    gramp::PresetManager presetManager;

    // smoothed continuous parameters (D-006: multiplicative for gain-like)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> inputGainSm, masterSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> boostSm, bassSm, midSm, trebleSm,
        presenceSm, depthSm, sagSm, micPosSm, irMixSm, postLowSm, postHighSm,
        gateThrSm, driveSm;

    int currentVoicing = 1;         // Plexi default (matches amp-voicing default)
    float loudnessGain = 1.0f;      // S-009 per-voicing trim, applied at the master stage
    int pendingVoicing = 1;
    int currentCab = 0, pendingCab = 0;
    int currentOs = 1, pendingOs = 1;   // High default
    bool currentPostEq = false;

    int crossfadeSamplesLeft = 0;   // 5 ms crossfade old-processed -> new-processed
    juce::AudioBuffer<float> prevBlock;  // previous block's final output
    bool hasPrevBlockOutput = false;     // false until the first block completes:
                                         // right after prepareToPlay there is no
                                         // previous output to crossfade against
                                         // (prevBlock starts cleared, never garbage)

    juce::AudioBuffer<float> dryScratch, postEqScratch;

    bool prepared = false;
    bool userIrApplied = false;
    double currentRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrimAmpAudioProcessor)
};
