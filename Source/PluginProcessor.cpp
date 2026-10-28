#include "PluginProcessor.h"

#if ! defined (GRIMAMP_NO_EDITOR)
#include "PluginEditor.h"
#endif

namespace
{
    inline float norm (const juce::AudioProcessorValueTreeState& apvts, const juce::StringRef id)
    {
        return apvts.getRawParameterValue (id)->load();
    }

    inline float dbToGain (float db) { return juce::Decibels::decibelsToGain (db, -120.0f); }

    /*  Choice parameters store their index NORMALISED as index/(count-1)
        (getRawParameterValue / getValue return 0..1), so an (int) cast of the
        raw value is wrong for count > 2 — e.g. voicing 5 -> (int)(5/6) = 0.
        Convert through the parameter's own range and round. */
    inline int choiceIndex (const juce::AudioProcessorValueTreeState& apvts, const juce::StringRef id)
    {
        auto* p = apvts.getParameter (id);
        return (int) std::lround (p->convertFrom0to1 (p->getValue()));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout GrimAmpAudioProcessor::createParameterLayout()
{
    using namespace gramp;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto addFloat = [&params] (const juce::String& id, const juce::String& name,
                               juce::NormalisableRange<float> range, float def,
                               const juce::String& label = {})
    {
        auto attrs = juce::AudioParameterFloatAttributes().withLabel (label).withAutomatable (true);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 },
                                                                       name, range, def, attrs));
    };

    auto addBool = [&params] (const juce::String& id, const juce::String& name, bool def)
    {
        auto attrs = juce::AudioParameterBoolAttributes().withAutomatable (true);
        params.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 },
                                                                      name, def, attrs));
    };

    auto addChoice = [&params] (const juce::String& id, const juce::String& name,
                                const juce::StringArray& choices, int def)
    {
        auto attrs = juce::AudioParameterChoiceAttributes().withAutomatable (true);
        params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, 1 },
                                                                        name, choices, def, attrs));
    };

    // Input / gate
    addFloat (param::inputGain,     "Input Gain",     { -24.0f, 24.0f, 0.1f }, 0.0f, "dB");
    addBool  (param::gateOn,        "Gate",           true);
    addFloat (param::gateThreshold, "Gate Threshold", { -80.0f, 0.0f, 0.5f },  -60.0f, "dB");

    // Boost
    addBool  (param::boostOn,       "Boost",          false);
    addFloat (param::boostAmount,   "Boost Amount",   { 0.0f, 1.0f, 0.001f }, 0.5f);

    // Preamp / tone / power
    addFloat (param::drive,         "Drive",          { 0.0f, 1.0f, 0.001f }, 0.5f);
    addFloat (param::bass,          "Bass",           { 0.0f, 1.0f, 0.001f }, 0.5f);
    addFloat (param::mid,           "Mid",            { 0.0f, 1.0f, 0.001f }, 0.5f);
    addFloat (param::treble,        "Treble",         { 0.0f, 1.0f, 0.001f }, 0.5f);
    addFloat (param::presence,      "Presence",       { 0.0f, 1.0f, 0.001f }, 0.5f);
    addFloat (param::depth,         "Depth",          { 0.0f, 1.0f, 0.001f }, 0.5f);
    addFloat (param::sag,           "Sag",            { 0.0f, 1.0f, 0.001f }, 0.5f);

    // Choices
    addChoice (param::ampVoicing,   "Amp Voicing",    juce::StringArray (voicing::names, voicing::count), 1);
    addChoice (param::cabIr,        "Cabinet IR",     juce::StringArray (cab::names, cab::count), 0);
    addChoice (param::oversampling, "Oversampling",   juce::StringArray (oversampling::names, oversampling::count), 1);

    // Cabinet shaping
    addFloat (param::micPosition,   "Mic Position",   { 0.0f, 1.0f, 0.001f }, 0.25f);
    addFloat (param::irMix,         "IR Mix",         { 0.0f, 1.0f, 0.001f }, 1.0f);

    // Post EQ
    addBool  (param::postEqOn,      "Post EQ",        false);
    addFloat (param::postEqLow,     "Post EQ Low",    { -12.0f, 12.0f, 0.1f }, 0.0f, "dB");
    addFloat (param::postEqHigh,    "Post EQ High",   { -12.0f, 12.0f, 0.1f }, 0.0f, "dB");

    // Output
    addFloat (param::master,        "Master",         { 0.0f, 1.0f, 0.001f }, 0.7f);

    return { params.begin(), params.end() };
}

GrimAmpAudioProcessor::GrimAmpAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout()),
      presetManager (apvts)
{
    // 20 ms ramps, reset in prepareToPlay (D-006)
    auto resetSmoothing = [] (auto& sm, float value)
    {
        sm.setCurrentAndTargetValue (value);
    };

    resetSmoothing (inputGainSm, 1.0f);
    resetSmoothing (masterSm, 0.7f);
    resetSmoothing (boostSm, 0.5f);
    resetSmoothing (bassSm, 0.5f);
    resetSmoothing (midSm, 0.5f);
    resetSmoothing (trebleSm, 0.5f);
    resetSmoothing (presenceSm, 0.5f);
    resetSmoothing (depthSm, 0.5f);
    resetSmoothing (sagSm, 0.5f);
    resetSmoothing (micPosSm, 0.25f);
    resetSmoothing (irMixSm, 1.0f);
    resetSmoothing (postLowSm, 0.0f);
    resetSmoothing (postHighSm, 0.0f);
    resetSmoothing (gateThrSm, -60.0f);
    resetSmoothing (driveSm, 0.5f);
}

const juce::String GrimAmpAudioProcessor::getName() const
{
#ifdef JucePlugin_Name
    return JucePlugin_Name;
#else
    return "GrimAmp";
#endif
}

bool GrimAmpAudioProcessor::acceptsMidi() const { return false; }
bool GrimAmpAudioProcessor::producesMidi() const { return false; }
bool GrimAmpAudioProcessor::isMidiEffect() const { return false; }
double GrimAmpAudioProcessor::getTailLengthSeconds() const { return 0.0; }

bool GrimAmpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();

    if (in != out)
        return false;
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void GrimAmpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentRate = sampleRate;

    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    gate.prepare (sampleRate, samplesPerBlock);
    boost.prepare (spec);
    cabIR.prepare (spec);
    postEq.prepare (spec);
    osStage.prepare (spec, pendingOs == gramp::oversampling::highIndex);

    const auto& row = gramp::voicings[currentVoicing];
    const double internalRate = osStage.getInternalSampleRate();
    preamp.prepare (internalRate, row);
    toneStack.prepare (internalRate, row);
    powerAmp.prepare (internalRate, row);
    loudnessGain = juce::Decibels::decibelsToGain (row.loudnessGainDb);

    dryScratch.setSize (2, samplesPerBlock);
    postEqScratch.setSize (2, samplesPerBlock);
    prevBlock.setSize (2, samplesPerBlock);
    prevBlock.clear();              // no previous output exists after prepare
    hasPrevBlockOutput = false;

    // 20 ms ramps (D-006)
    auto ramp = [&] (auto& sm) { sm.reset (sampleRate, 0.020); };
    ramp (inputGainSm);  ramp (masterSm);   ramp (boostSm);    ramp (bassSm);
    ramp (midSm);        ramp (trebleSm);   ramp (presenceSm); ramp (depthSm);
    ramp (sagSm);        ramp (micPosSm);   ramp (irMixSm);    ramp (postLowSm);
    ramp (postHighSm);   ramp (gateThrSm);  ramp (driveSm);

    cabIR.setMicPosition (micPosSm.getCurrentValue());
    cabIR.setMix (irMixSm.getCurrentValue());
    cabIR.selectCab (pendingCab);
    cabIR.applyPending();     // load the default IR now so the first block is correct

    updateLatency();
    prepared = true;
}

void GrimAmpAudioProcessor::releaseResources()
{
    prepared = false;
    hasPrevBlockOutput = false;
}

void GrimAmpAudioProcessor::updateLatency()
{
    setLatencySamples (osStage.getLatency());
}

juce::AudioBuffer<float>& GrimAmpAudioProcessor::getScratch (int numChannels, int numSamples)
{
    dryScratch.setSize (juce::jmax (1, juce::jmin (2, numChannels)), numSamples, false, false, true);
    return dryScratch;
}

void GrimAmpAudioProcessor::pullSmoothParams (int numSamples)
{
    inputGainSm.setTargetValue (dbToGain (norm (apvts, gramp::param::inputGain)));
    masterSm.setTargetValue (juce::jmax (0.0001f, norm (apvts, gramp::param::master)));
    boostSm.setTargetValue (norm (apvts, gramp::param::boostAmount));
    bassSm.setTargetValue (norm (apvts, gramp::param::bass));
    midSm.setTargetValue (norm (apvts, gramp::param::mid));
    trebleSm.setTargetValue (norm (apvts, gramp::param::treble));
    presenceSm.setTargetValue (norm (apvts, gramp::param::presence));
    depthSm.setTargetValue (norm (apvts, gramp::param::depth));
    sagSm.setTargetValue (norm (apvts, gramp::param::sag));
    micPosSm.setTargetValue (norm (apvts, gramp::param::micPosition));
    irMixSm.setTargetValue (norm (apvts, gramp::param::irMix));
    postLowSm.setTargetValue (norm (apvts, gramp::param::postEqLow));
    postHighSm.setTargetValue (norm (apvts, gramp::param::postEqHigh));
    gateThrSm.setTargetValue (norm (apvts, gramp::param::gateThreshold));
    driveSm.setTargetValue (norm (apvts, gramp::param::drive));

    juce::ignoreUnused (numSamples);
}

void GrimAmpAudioProcessor::applySteppedParams()
{
    // raw atomic reads at block top (S-012); switches apply at block boundary
    pendingVoicing = choiceIndex (apvts, gramp::param::ampVoicing);
    pendingCab     = choiceIndex (apvts, gramp::param::cabIr);
    pendingOs      = choiceIndex (apvts, gramp::param::oversampling);

    bool switched = false;

    if (pendingOs != currentOs)
    {
        currentOs = pendingOs;
        const juce::dsp::ProcessSpec spec { currentRate, (juce::uint32) getBlockSize(), 2 };
        osStage.prepare (spec, currentOs == gramp::oversampling::highIndex);
        const double internalRate = osStage.getInternalSampleRate();
        const auto& row = gramp::voicings[currentVoicing];
        preamp.prepare (internalRate, row);
        toneStack.prepare (internalRate, row);
        powerAmp.prepare (internalRate, row);
        osStage.reset();
        updateLatency();
        switched = true;
    }

    if (pendingVoicing != currentVoicing)
    {
        currentVoicing = juce::jlimit (0, gramp::numVoicings - 1, pendingVoicing);
        const double internalRate = osStage.getInternalSampleRate();
        const auto& row = gramp::voicings[currentVoicing];
        preamp.setVoicing (internalRate, row, driveSm.getCurrentValue());
        toneStack.setVoicing (internalRate, row);
        powerAmp.setVoicing (internalRate, row);
        loudnessGain = juce::Decibels::decibelsToGain (row.loudnessGainDb);
        switched = true;
    }

    if (pendingCab != currentCab)
    {
        currentCab = pendingCab;
        cabIR.selectCab (currentCab);
        switched = true;
    }

    cabIR.applyPending();   // wait-free IR swap at block boundary

    // user IR handoff from the loader thread
    juce::AudioBuffer<float> loaded;
    double loadedRate = 0.0;
    if (userIrLoader.takeLoadedBuffer (loaded, loadedRate))
    {
        cabIR.setUserImpulseResponse (std::move (loaded), loadedRate);
        userIrApplied = true;
    }

    if (switched)
        crossfadeSamplesLeft = (int) (currentRate * 0.005);   // 5 ms (D-006/S-012)
}

void GrimAmpAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin (buffer.getNumChannels(), 2);
    if (numSamples == 0 || numChannels == 0 || ! prepared)
        return;

    pullSmoothParams (numSamples);
    applySteppedParams();

    // ---- stepped-change crossfade: blend against the PREVIOUS block's
    // processed output (old settings), not the raw input ----
    const bool doCrossfade = crossfadeSamplesLeft > 0 && hasPrevBlockOutput
                             && prevBlock.getNumSamples() >= numSamples
                             && prevBlock.getNumChannels() >= numChannels;

    // ---- input gain ----
    {
        auto* ch0 = buffer.getWritePointer (0);
        const float g0 = inputGainSm.getNextValue();
        juce::FloatVectorOperations::multiply (ch0, g0, numSamples);
        if (numChannels > 1)
        {
            inputGainSm.skip (numSamples - 1);
            auto* ch1 = buffer.getWritePointer (1);
            const float g1 = inputGainSm.getNextValue();
            juce::FloatVectorOperations::multiply (ch1, g1, numSamples);
        }
        else
        {
            inputGainSm.skip (numSamples - 1);
        }
    }

    // ---- noise gate ----
    if (norm (apvts, gramp::param::gateOn) >= 0.5f)
    {
        gate.setSmoothedThreshold (gateThrSm.getNextValue());
        gateThrSm.skip (numSamples - 1);
        gate.processBlock (buffer.getWritePointer (0), numSamples);
        if (numChannels > 1)
            gate.processBlock (buffer.getWritePointer (1), numSamples);
    }

    // ---- boost ----
    if (norm (apvts, gramp::param::boostOn) >= 0.5f)
    {
        boost.setAmount (boostSm.getNextValue());
        boostSm.skip (numSamples - 1);
        auto block = juce::dsp::AudioBlock<float> (buffer);
        boost.processBlock (block);
    }

    // ---- amp region (4x oversampled in High mode) ----
    {
        auto block = juce::dsp::AudioBlock<float> (buffer);
        const float drive = driveSm.getNextValue();
        driveSm.skip (numSamples - 1);

        // advance remaining smoothed values once per block (block-rate for filters,
        // per-sample only where zipper noise matters — see validation/automation-audit.md)
        const float bass = bassSm.getNextValue();    bassSm.skip (numSamples - 1);
        const float mid = midSm.getNextValue();      midSm.skip (numSamples - 1);
        const float treb = trebleSm.getNextValue();  trebleSm.skip (numSamples - 1);
        const float pres = presenceSm.getNextValue();presenceSm.skip (numSamples - 1);
        const float dep = depthSm.getNextValue();    depthSm.skip (numSamples - 1);

        osStage.process (block, [&] (juce::dsp::AudioBlock<float>& osBlock, double osRate)
        {
            juce::ignoreUnused (osRate);
            preamp.setDrive (drive);
            toneStack.setBandGains (bass, mid, treb, pres, dep);
            powerAmp.setSag (sagSm.getCurrentValue());

            for (size_t ch = 0; ch < osBlock.getNumChannels(); ++ch)
            {
                auto* d = osBlock.getChannelPointer (ch);
                const int n = (int) osBlock.getNumSamples();
                preamp.process (d, n);
                auto oneCh = juce::dsp::AudioBlock<float> (&d, 1, (size_t) n);
                toneStack.processBlock (oneCh);
                powerAmp.process (d, n);
            }

            powerAmp.processThump (osBlock, sagSm.getCurrentValue());
        });
    }

    // ---- cabinet IR ----
    cabIR.setMicPosition (micPosSm.getNextValue());
    micPosSm.skip (numSamples - 1);
    cabIR.setMix (irMixSm.getNextValue());
    irMixSm.skip (numSamples - 1);
    {
        auto block = juce::dsp::AudioBlock<float> (buffer);
        auto& dry = getScratch (numChannels, numSamples);
        cabIR.process (block, dry);
    }

    // ---- post EQ ----
    const bool postEqOn = norm (apvts, gramp::param::postEqOn) >= 0.5f;
    if (postEqOn != currentPostEq)
    {
        currentPostEq = postEqOn;
        crossfadeSamplesLeft = juce::jmax (crossfadeSamplesLeft, (int) (currentRate * 0.005));
    }
    if (currentPostEq)
    {
        const float lowDb = postLowSm.getNextValue();   postLowSm.skip (numSamples - 1);
        const float highDb = postHighSm.getNextValue(); postHighSm.skip (numSamples - 1);
        auto block = juce::dsp::AudioBlock<float> (buffer);
        postEq.processBlock (block, lowDb, highDb);
    }

    // ---- master (+ S-009 per-voicing loudness trim) ----
    {
        const float lg = loudnessGain;
        const float m0 = masterSm.getNextValue() * lg;
        juce::FloatVectorOperations::multiply (buffer.getWritePointer (0), m0, numSamples);
        masterSm.skip (numSamples - 1);
        if (numChannels > 1)
        {
            const float m1 = masterSm.getNextValue() * lg;
            juce::FloatVectorOperations::multiply (buffer.getWritePointer (1), m1, numSamples);
        }
    }

    // ---- 5 ms crossfade after stepped changes ----
    if (doCrossfade)
    {
        const int n = juce::jmin (numSamples, crossfadeSamplesLeft);
        const float inc = 1.0f / juce::jmax (1, (int) (currentRate * 0.005));
        auto* cur0 = buffer.getWritePointer (0);
        auto* prev0 = prevBlock.getReadPointer (0);
        for (int i = 0; i < n; ++i)
        {
            const float t = 1.0f - (crossfadeSamplesLeft - i) * inc;
            cur0[i] = t * cur0[i] + (1.0f - t) * prev0[i];
        }
        if (numChannels > 1)
        {
            auto* cur1 = buffer.getWritePointer (1);
            auto* prev1 = prevBlock.getReadPointer (1);
            for (int i = 0; i < n; ++i)
            {
                const float t = 1.0f - (crossfadeSamplesLeft - i) * inc;
                cur1[i] = t * cur1[i] + (1.0f - t) * prev1[i];
            }
        }
        crossfadeSamplesLeft -= n;
    }

    // remember this block's final output for a possible crossfade next block
    prevBlock.setSize (numChannels, numSamples, false, false, true);
    for (int ch = 0; ch < numChannels; ++ch)
        prevBlock.copyFrom (ch, 0, buffer, ch, 0, numSamples);
    hasPrevBlockOutput = true;
}

void GrimAmpAudioProcessor::requestUserIrLoad (const juce::File& file)
{
    userIrLoader.loadFileAsync (file, currentRate, [] (bool) {});
}

bool GrimAmpAudioProcessor::isUserIrLoading() const noexcept
{
    return userIrLoader.isBusy();
}

bool GrimAmpAudioProcessor::hasEditor() const
{
#if defined (GRIMAMP_NO_EDITOR)
    return false;
#else
    return true;
#endif
}

juce::AudioProcessorEditor* GrimAmpAudioProcessor::createEditor()
{
#if defined (GRIMAMP_NO_EDITOR)
    return nullptr;
#else
    return new GrimAmpAudioProcessorEditor (*this);
#endif
}

int GrimAmpAudioProcessor::getNumPrograms()    { return 1; }
int GrimAmpAudioProcessor::getCurrentProgram() { return 0; }
void GrimAmpAudioProcessor::setCurrentProgram (int) {}
const juce::String GrimAmpAudioProcessor::getProgramName (int) { return {}; }
void GrimAmpAudioProcessor::changeProgramName (int, const juce::String&) {}

void GrimAmpAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto xml = state.createXml();
    copyXmlToBinary (*xml, destData);
}

void GrimAmpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (auto newState = juce::ValueTree::fromXml (*xml); newState.isValid())
        {
            apvts.replaceState (newState);

            // keep stepped mirrors in sync for the next block boundary
            currentVoicing = pendingVoicing = choiceIndex (apvts, gramp::param::ampVoicing);
            currentCab     = pendingCab     = choiceIndex (apvts, gramp::param::cabIr);
            currentOs      = pendingOs      = choiceIndex (apvts, gramp::param::oversampling);
            loudnessGain   = juce::Decibels::decibelsToGain (
                gramp::voicings[juce::jlimit (0, gramp::numVoicings - 1, currentVoicing)].loudnessGainDb);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrimAmpAudioProcessor();
}
