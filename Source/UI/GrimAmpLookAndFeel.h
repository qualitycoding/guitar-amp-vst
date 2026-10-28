/*
  GrimAmpLookAndFeel — flat modern dark theme, vector rotary knobs (A-011).
  No raster assets; everything is drawn with Paths/Colours.
*/
#pragma once

namespace gramp
{
class GrimAmpLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GrimAmpLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId,Colours::panel);
        setColour (juce::Slider::backgroundColourId,           Colours::panel);
        setColour (juce::Slider::trackColourId,                Colours::accent);
        setColour (juce::Slider::thumbColourId,                Colours::knob);
        setColour (juce::Slider::textBoxTextColourId,          Colours::text);
        setColour (juce::Slider::textBoxBackgroundColourId,    Colours::panel.brighter (0.1f));
        setColour (juce::Slider::textBoxOutlineColourId,       juce::Colours::transparentBlack);
        setColour (juce::ToggleButton::textColourId,           Colours::text);
        setColour (juce::ToggleButton::tickColourId,           Colours::accent);
        setColour (juce::ToggleButton::tickDisabledColourId,   Colours::textDisabled);
        setColour (juce::ComboBox::backgroundColourId,         Colours::panel.brighter (0.1f));
        setColour (juce::ComboBox::textColourId,               Colours::text);
        setColour (juce::ComboBox::outlineColourId,            juce::Colours::transparentBlack);
        setColour (juce::PopupMenu::backgroundColourId,        Colours::panel.brighter (0.05f));
        setColour (juce::PopupMenu::textColourId,              Colours::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::accent);
        setColour (juce::PopupMenu::highlightedTextColourId,   juce::Colours::white);
        setColour (juce::Label::textColourId,                  Colours::text);
        setColour (juce::GroupComponent::textColourId,         Colours::groupTitle);
        setColour (juce::GroupComponent::outlineColourId,      Colours::panel.brighter (0.25f));
        setColour (juce::TextButton::buttonColourId,           Colours::panel.brighter (0.15f));
        setColour (juce::TextButton::buttonOnColourId,         Colours::accent);
        setColour (juce::TextButton::textColourOffId,          Colours::text);
        setColour (juce::TextButton::textColourOnId,           juce::Colours::white);
        setColour (juce::TextEditor::backgroundColourId,       Colours::panel.brighter (0.1f));
        setColour (juce::TextEditor::textColourId,             Colours::text);
        setColour (juce::TextEditor::outlineColourId,          juce::Colours::transparentBlack);
    }

    struct Colours
    {
        inline static const juce::Colour window      { 0xff17191d };
        inline static const juce::Colour panel       { 0xff1f232a };
        inline static const juce::Colour accent      { 0xffe8552f };
        inline static const juce::Colour knob        { 0xff2d333c };
        inline static const juce::Colour text        { 0xffd7dae0 };
        inline static const juce::Colour textDisabled{ 0xff6b7078 };
        inline static const juce::Colour groupTitle  { 0xff8b909a };
    };

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float centreX = bounds.getCentreX();
        const float centreY = bounds.getCentreY();
        const float knobRadius = radius * 0.72f;

        // track arc
        juce::Path track;
        track.addCentredArc (centreX, centreY, radius * 0.92f, radius * 0.92f, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (Colours::panel.brighter (0.35f));
        g.strokePath (track, juce::PathStrokeType (radius * 0.10f,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        // value arc
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path value;
        value.addCentredArc (centreX, centreY, radius * 0.92f, radius * 0.92f, 0.0f,
                             rotaryStartAngle, angle, true);
        g.setColour (Colours::accent);
        g.strokePath (value, juce::PathStrokeType (radius * 0.10f,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        // knob body
        juce::Path knob;
        knob.addEllipse (centreX - knobRadius, centreY - knobRadius,
                         knobRadius * 2.0f, knobRadius * 2.0f);
        g.setColour (Colours::knob);
        g.fillPath (knob);
        g.setColour (Colours::panel.brighter (0.4f));
        g.strokePath (knob, juce::PathStrokeType (1.2f));

        // pointer
        juce::Path pointer;
        const float pr = knobRadius * 0.85f;
        pointer.addRoundedRectangle (-radius * 0.045f, -pr, radius * 0.09f, pr * 0.9f,
                                     radius * 0.04f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle, centreX, centreY)
                                    .translated (centreX, centreY));
        g.setColour (Colours::text);
        g.fillPath (pointer);

        ignoreUnused (slider);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrimAmpLookAndFeel)
};

} // namespace gramp
