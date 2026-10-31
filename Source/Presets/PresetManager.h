/*
  PresetManager — factory + user presets (S-011, D-007, D-008).

  Factory presets ship as BinaryData XML (APVTS state snapshots); user presets
  are ".grindpreset" XML files in the OS app-data dir, created on first use.

  Gesture discipline (D-007): preset loads go through APVTS replaceState only.
  This class NEVER calls setValueNotifyingHost/beginChangeGesture itself; the
  unit test (T-011) spies on that contract via a mock notifier interface.
*/
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <memory>
#include <vector>

namespace gramp
{
/*  Injectable seam for T-011: production code routes state through
    replaceState; tests record calls on a mock. */
class PresetStatePort
{
public:
    virtual ~PresetStatePort() = default;
    virtual void replaceState (const juce::ValueTree& newState) = 0;
    virtual void setValueNotifyingHost (const juce::String& /*paramID*/, float /*value*/)
    {
        // deliberately non-pure: the mock counts calls; production never calls this
    }
};

class ApvtsPresetPort : public PresetStatePort
{
public:
    explicit ApvtsPresetPort (juce::AudioProcessorValueTreeState& stateIn) : apvts (stateIn) {}
    void replaceState (const juce::ValueTree& newState) override { apvts.replaceState (newState); }

private:
    juce::AudioProcessorValueTreeState& apvts;
};

class PresetManager
{
public:
    struct PresetInfo
    {
        juce::String name;
        bool isFactory = true;
        int factoryIndex = -1;
        juce::File userFile;
    };

    explicit PresetManager (juce::AudioProcessorValueTreeState& apvts);

    /*  Takes the state-application port (normally ApvtsPresetPort). Tests
        inject a recording mock (T-011). */
    void setStatePort (std::unique_ptr<PresetStatePort> port);

    int getNumPresets() const;
    PresetInfo getPresetInfo (int index) const;
    juce::StringArray getPresetNames() const;

    /*  Loads a preset by list index. Returns false if the preset could not be
        parsed (tolerant parsing: unknown attributes ignored, missing ones keep
        current values — D-008). Never notifies the host per parameter. */
    bool loadPreset (int index);

    /*  Loads preset XML from a string (used by T-008/T-013 fuzz corpus). */
    bool loadPresetFromXml (const juce::String& xmlText);

    /*  Saves the current APVTS state as a user preset. Returns the new index
        or -1 on failure. */
    int saveUserPreset (const juce::String& name);

    /*  Rescans the user preset directory. Call after external file changes. */
    void refresh();

    juce::File getUserPresetsDir() const;

    static juce::ValueTree tolerantParse (const juce::String& xmlText);

private:
    void rebuildList();

    juce::AudioProcessorValueTreeState& apvts;
    std::unique_ptr<PresetStatePort> statePort;
    std::vector<PresetInfo> presets;
    int numFactory = 0;

    JUCE_DECLARE_NON_COPYABLE (PresetManager)
};

} // namespace gramp
