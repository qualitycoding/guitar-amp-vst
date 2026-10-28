/*
  GrimAmpAudioProcessorEditor — amp-section layout with a preset browser bar
  (S-010). Every control is bound to its APVTS parameter via an attachment
  (this is what makes gesture discipline natural, T-011). Choice parameters
  are non-editable combo boxes (K-20261009T164200Z / K-20261009T164300Z).
*/
#pragma once
#include "UI/GrimAmpLookAndFeel.h"

class GrimAmpAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GrimAmpAudioProcessorEditor (GrimAmpAudioProcessor&);
    ~GrimAmpAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<APVTS::SliderAttachment> attachment;
    };

    Knob& addKnob (juce::Component& parent, const juce::String& paramId,
                   const juce::String& name);

    GrimAmpAudioProcessor& processor;
    gramp::GrimAmpLookAndFeel laf;

    // preset bar
    juce::TextButton prevPreset { "<" }, nextPreset { ">" }, savePreset { "save" };
    juce::ComboBox presetBox;
    juce::Label presetNameEditor;
    std::unique_ptr<APVTS::ComboBoxAttachment> presetBoxAttachment;   // unused slot kept for clarity
    int currentPresetIndex = 0;

    // sections
    juce::GroupComponent inputGroup, boostGroup, preampGroup, powerGroup,
                         toneGroup, cabGroup, outputGroup;

    Knob inputGain, gateThreshold;
    juce::ToggleButton gateToggle;
    std::unique_ptr<APVTS::ButtonAttachment> gateAttachment;

    Knob boostAmount;
    juce::ToggleButton boostToggle;
    std::unique_ptr<APVTS::ButtonAttachment> boostAttachment;

    Knob drive, sag;

    Knob bass, mid, treble, presence, depth;

    juce::ComboBox voicingBox, cabBox, oversamplingBox;
    std::unique_ptr<APVTS::ComboBoxAttachment> voicingAttachment, cabAttachment,
                                               oversamplingAttachment;
    Knob micPosition, irMix;

    juce::ToggleButton postEqToggle;
    std::unique_ptr<APVTS::ButtonAttachment> postEqAttachment;
    Knob postEqLow, postEqHigh, master;

    std::vector<std::unique_ptr<Knob>> ownedKnobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrimAmpAudioProcessorEditor)
};
