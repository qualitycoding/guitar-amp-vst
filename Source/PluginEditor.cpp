#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::Slider::SliderStyle rotary()
    {
        return juce::Slider::RotaryVerticalDrag;
    }

    void styleLabel (juce::Label& label)
    {
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::FontOptions (12.0f));
    }
}

GrimAmpAudioProcessorEditor::GrimAmpAudioProcessorEditor (GrimAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&laf);

    // ---- preset bar ----
    addAndMakeVisible (prevPreset);
    addAndMakeVisible (nextPreset);
    addAndMakeVisible (savePreset);
    addAndMakeVisible (presetBox);
    addAndMakeVisible (presetNameEditor);
    presetNameEditor.setText ("my preset", juce::dontSendNotification);
    presetNameEditor.setEditable (true);

    presetBox.addItemList (processor.getPresetManager().getPresetNames(), 1);
    if (presetBox.getNumItems() > 0)
        presetBox.setSelectedItemIndex (0, juce::dontSendNotification);

    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedItemIndex();
        if (idx >= 0)
        {
            currentPresetIndex = idx;
            processor.getPresetManager().loadPreset (idx);
        }
    };
    prevPreset.onClick = [this]
    {
        const int n = processor.getPresetManager().getNumPresets();
        if (n > 0)
        {
            currentPresetIndex = (currentPresetIndex + n - 1) % n;
            presetBox.setSelectedItemIndex (currentPresetIndex, juce::sendNotificationSync);
        }
    };
    nextPreset.onClick = [this]
    {
        const int n = processor.getPresetManager().getNumPresets();
        if (n > 0)
        {
            currentPresetIndex = (currentPresetIndex + 1) % n;
            presetBox.setSelectedItemIndex (currentPresetIndex, juce::sendNotificationSync);
        }
    };
    savePreset.onClick = [this]
    {
        auto name = presetNameEditor.getText().trim();
        if (name.isEmpty())
            name = "preset " + juce::String (processor.getPresetManager().getNumPresets() + 1);
        processor.getPresetManager().saveUserPreset (name);
        presetBox.clear (juce::dontSendNotification);
        presetBox.addItemList (processor.getPresetManager().getPresetNames(), 1);
        presetBox.setSelectedItemIndex (processor.getPresetManager().getNumPresets() - 1,
                                        juce::dontSendNotification);
    };

    // ---- groups ----
    auto setupGroup = [] (juce::GroupComponent& g, const juce::String& name)
    {
        g.setText (name);
        g.setTextLabelPosition (juce::Justification::centred);
    };

    setupGroup (inputGroup,  "Input / Gate");
    setupGroup (boostGroup,  "Boost");
    setupGroup (preampGroup, "Preamp");
    setupGroup (powerGroup,  "Power");
    setupGroup (toneGroup,   "Tone");
    setupGroup (cabGroup,    "Cabinet");
    setupGroup (outputGroup, "Output");

    for (auto* g : { &inputGroup, &boostGroup, &preampGroup, &powerGroup,
                     &toneGroup, &cabGroup, &outputGroup })
        addAndMakeVisible (*g);

    auto& apvts = processor.apvts;

    // Input / Gate
    addKnob (inputGroup, gramp::param::inputGain,     "Input");
    addKnob (inputGroup, gramp::param::gateThreshold, "Thr");
    inputGroup.addAndMakeVisible (gateToggle);
    gateToggle.setButtonText ("Gate");
    gateAttachment = std::make_unique<APVTS::ButtonAttachment> (apvts, gramp::param::gateOn, gateToggle);

    // Boost
    addKnob (boostGroup, gramp::param::boostAmount, "Amount");
    boostGroup.addAndMakeVisible (boostToggle);
    boostToggle.setButtonText ("Boost");
    boostAttachment = std::make_unique<APVTS::ButtonAttachment> (apvts, gramp::param::boostOn, boostToggle);

    // Preamp (voicing + drive)
    voicingBox.addItemList (juce::StringArray (gramp::voicing::names, gramp::voicing::count), 1);
    preampGroup.addAndMakeVisible (voicingBox);
    voicingAttachment = std::make_unique<APVTS::ComboBoxAttachment> (apvts, gramp::param::ampVoicing, voicingBox);
    addKnob (preampGroup, gramp::param::drive, "Drive");

    // Power
    addKnob (powerGroup, gramp::param::sag, "Sag");

    // Tone
    addKnob (toneGroup, gramp::param::bass,     "Bass");
    addKnob (toneGroup, gramp::param::mid,      "Mid");
    addKnob (toneGroup, gramp::param::treble,   "Treble");
    addKnob (toneGroup, gramp::param::presence, "Presence");
    addKnob (toneGroup, gramp::param::depth,    "Depth");

    // Cabinet
    cabBox.addItemList (juce::StringArray (gramp::cab::names, gramp::cab::count), 1);
    cabGroup.addAndMakeVisible (cabBox);
    cabAttachment = std::make_unique<APVTS::ComboBoxAttachment> (apvts, gramp::param::cabIr, cabBox);
    addKnob (cabGroup, gramp::param::micPosition, "Mic");
    addKnob (cabGroup, gramp::param::irMix,       "Mix");

    // Output
    outputGroup.addAndMakeVisible (postEqToggle);
    postEqToggle.setButtonText ("Post EQ");
    postEqAttachment = std::make_unique<APVTS::ButtonAttachment> (apvts, gramp::param::postEqOn, postEqToggle);
    addKnob (outputGroup, gramp::param::postEqLow,  "Low");
    addKnob (outputGroup, gramp::param::postEqHigh, "High");
    addKnob (outputGroup, gramp::param::master,     "Master");

    oversamplingBox.addItemList (juce::StringArray (gramp::oversampling::names,
                                                    gramp::oversampling::count), 1);
    oversamplingBox.setTooltip ("Eco: zero added latency, host-rate processing.\n"
                                "High: 4x oversampling (about 3 ms latency at 48 kHz), cleaner high gain.");
    outputGroup.addAndMakeVisible (oversamplingBox);
    oversamplingAttachment = std::make_unique<APVTS::ComboBoxAttachment> (
        apvts, gramp::param::oversampling, oversamplingBox);

    setResizable (true, true);
    setResizeLimits (800, 450, 1600, 1000);
    setSize (940, 600);
}

GrimAmpAudioProcessorEditor::~GrimAmpAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

GrimAmpAudioProcessorEditor::Knob& GrimAmpAudioProcessorEditor::addKnob (
    juce::Component& parent, const juce::String& paramId, const juce::String& name)
{
    auto k = std::make_unique<Knob>();
    auto& ref = *k;

    k->slider.setSliderStyle (rotary());
    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    k->label.setText (name, juce::dontSendNotification);
    styleLabel (k->label);
    k->attachment = std::make_unique<APVTS::SliderAttachment> (processor.apvts, paramId, k->slider);

    parent.addAndMakeVisible (k->slider);
    parent.addAndMakeVisible (k->label);
    ownedKnobs.push_back (std::move (k));
    return ref;
}

void GrimAmpAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (gramp::GrimAmpLookAndFeel::Colours::window);
}

void GrimAmpAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    // preset bar
    auto bar = bounds.removeFromTop (36);
    prevPreset.setBounds (bar.removeFromLeft (36).reduced (2));
    nextPreset.setBounds (bar.removeFromLeft (36).reduced (2));
    savePreset.setBounds (bar.removeFromRight (64).reduced (2));
    presetNameEditor.setBounds (bar.removeFromRight (140).reduced (4));
    presetBox.setBounds (bar.reduced (4, 4));

    bounds.removeFromTop (8);
    const int rowH = (bounds.getHeight() - 10) / 2;

    // top row: Input/Gate | Boost | Preamp | Power
    auto top = bounds.removeFromTop (rowH);
    auto makeCol = [&] (juce::GroupComponent& g)
    {
        auto col = top.removeFromLeft (getWidth() / 4 - 8).reduced (4);
        g.setBounds (col);
        return col;
    };

    auto lay2 = [] (juce::GroupComponent& g, Knob& a, Knob& b, juce::Button* toggle = nullptr)
    {
        auto r = g.getLocalBounds().reduced (10, 22);
        if (toggle != nullptr)
        {
            auto tr = r.removeFromBottom (26);
            toggle->setBounds (tr.withSizeKeepingCentre (90, 22));
        }
        a.slider.setBounds (r.removeFromLeft (r.getWidth() / 2).reduced (2));
        b.slider.setBounds (r.reduced (2));
        a.label.attachToComponent (&a.slider, false);
        a.label.setBounds (a.slider.getX(), a.slider.getY() - 16, a.slider.getWidth(), 16);
        b.label.setBounds (b.slider.getX(), b.slider.getY() - 16, b.slider.getWidth(), 16);
    };

    {
        auto col = makeCol (inputGroup);
        lay2 (inputGroup, inputGain, gateThreshold, &gateToggle);
        ignoreUnused (col);
    }
    {
        auto col = makeCol (boostGroup);
        auto r = boostGroup.getLocalBounds().reduced (10, 22);
        boostToggle.setBounds (r.removeFromBottom (26).withSizeKeepingCentre (90, 22));
        boostAmount.slider.setBounds (r.reduced (20, 2));
        boostAmount.label.setBounds (boostAmount.slider.getX(), boostAmount.slider.getY() - 16,
                                     boostAmount.slider.getWidth(), 16);
        ignoreUnused (col);
    }
    {
        auto col = makeCol (preampGroup);
        auto r = preampGroup.getLocalBounds().reduced (10, 22);
        voicingBox.setBounds (r.removeFromTop (26).reduced (8, 0));
        drive.slider.setBounds (r.reduced (20, 2));
        drive.label.setBounds (drive.slider.getX(), drive.slider.getY() - 16,
                               drive.slider.getWidth(), 16);
        ignoreUnused (col);
    }
    {
        auto col = makeCol (powerGroup);
        auto r = powerGroup.getLocalBounds().reduced (10, 22);
        sag.slider.setBounds (r.reduced (20, 2));
        sag.label.setBounds (sag.slider.getX(), sag.slider.getY() - 16,
                             sag.slider.getWidth(), 16);
        ignoreUnused (col);
    }

    // bottom row: Tone (wider) | Cabinet | Output
    auto bottom = bounds.reduced (0, 4);
    toneGroup.setBounds (bottom.removeFromLeft (juce::roundToInt (getWidth() * 0.42f)).reduced (4));
    cabGroup.setBounds (bottom.removeFromLeft (juce::roundToInt (getWidth() * 0.28f)).reduced (4));
    outputGroup.setBounds (bottom.reduced (4));

    auto lay5 = [] (juce::GroupComponent& g, juce::Array<Knob*> knobs)
    {
        auto r = g.getLocalBounds().reduced (12, 22);
        const int w = r.getWidth() / knobs.size();
        for (auto* k : knobs)
        {
            auto cell = r.removeFromLeft (w).reduced (2);
            k->label.setBounds (cell.removeFromTop (16));
            k->slider.setBounds (cell);
        }
    };

    lay5 (toneGroup, { &bass, &mid, &treble, &presence, &depth });

    {
        auto r = cabGroup.getLocalBounds().reduced (12, 22);
        cabBox.setBounds (r.removeFromTop (26).reduced (4, 0));
        const int w = r.getWidth() / 2;
        micPosition.label.setBounds (r.removeFromLeft (w).removeFromTop (16));
        micPosition.slider.setBounds (r.removeFromLeft (w).reduced (2));
        // second half
        auto r2 = cabGroup.getLocalBounds().reduced (12, 22);
        r2.removeFromTop (26);
        irMix.label.setBounds (r2.removeFromRight (w).removeFromTop (16));
        irMix.slider.setBounds (r2.removeFromRight (w).reduced (2));
    }

    {
        auto r = outputGroup.getLocalBounds().reduced (12, 22);
        auto toggles = r.removeFromBottom (28);
        postEqToggle.setBounds (toggles.removeFromLeft (toggles.getWidth() / 2).withSizeKeepingCentre (96, 24));
        oversamplingBox.setBounds (toggles.withTrimmedLeft (8).reduced (4, 2));
        const int w = r.getWidth() / 3;
        postEqLow.label.setBounds (r.removeFromLeft (w).removeFromTop (16));
        postEqLow.slider.setBounds (r.removeFromLeft (w).reduced (2));
        auto r3 = outputGroup.getLocalBounds().reduced (12, 22);
        r3.removeFromBottom (28);
        postEqHigh.label.setBounds (r3.removeFromRight (w).removeFromTop (16));
        postEqHigh.slider.setBounds (r3.removeFromRight (w).reduced (2));
        auto r4 = outputGroup.getLocalBounds().reduced (12, 22);
        r4.removeFromBottom (28);
        r4.removeFromLeft (w); r4.removeFromRight (w);
        master.label.setBounds (r4.removeFromTop (16));
        master.slider.setBounds (r4.reduced (6, 0));
    }
}
