#pragma once
#include <JuceHeader.h>
#include "ProVocalLookAndFeel.h"

class PresetBrowser : public juce::Component
{
public:
    PresetBrowser(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics&) override;
    void resized() override;

    void loadPreset(const juce::File& file);
    void savePreset(const juce::File& file);
    void refreshPresetList();

private:
    juce::AudioProcessorValueTreeState& apvtsRef;

    juce::ComboBox presetCombo;
    juce::TextButton saveButton { "Save" };
    juce::TextButton saveAsButton { "Save As" };
    juce::TextButton deleteButton { "Delete" };

    juce::File presetDirectory;
    juce::StringArray presetNames;
    juce::Array<juce::File> presetFiles;

    void loadSelectedPreset();
    void saveCurrentPreset();
    void savePresetAs();
    void deleteSelectedPreset();

    juce::File getPresetDirectory() const;
};
