#pragma once
#include <JuceHeader.h>
#include "../Utils/ScaleDefinitions.h"
#include "ProVocalLookAndFeel.h"

class ScaleSelector : public juce::Component
{
public:
    ScaleSelector(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::ComboBox rootNoteCombo;
    juce::ComboBox scaleCombo;

    std::unique_ptr<ComboAttachment> rootNoteAttach;
    std::unique_ptr<ComboAttachment> scaleAttach;

    // 12 note bypass toggle buttons
    std::array<juce::ToggleButton, 12> noteButtons;
    std::array<std::unique_ptr<ButtonAttachment>, 12> noteAttachments;

    juce::Label rootLabel, scaleLabel;
};
