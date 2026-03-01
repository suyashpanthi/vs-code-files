#pragma once
#include "PluginProcessor.h"
#include "GUI/ProVocalLookAndFeel.h"
#include "GUI/LevelMeter.h"
#include "GUI/PitchDisplayComponent.h"
#include "GUI/ScaleSelector.h"
#include "GUI/PresetBrowser.h"
#include "GUI/TabPanel.h"

// ============================================================================
// Tab Content Components
// ============================================================================

class PitchTabContent : public juce::Component
{
public:
    PitchTabContent(ProVocalProcessor& p);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    PitchDisplayComponent pitchDisplay;
    ScaleSelector scaleSelector;

    juce::ToggleButton pitchOnButton { "Enable" };
    std::unique_ptr<ButtonAttachment> pitchOnAttach;

    juce::ToggleButton midiModeButton { "MIDI Mode" };
    std::unique_ptr<ButtonAttachment> midiModeAttach;

    juce::Slider retuneSpeedKnob, humanizeKnob, vibratoRateKnob, vibratoDepthKnob;
    juce::Slider formantShiftKnob, bendRangeKnob;
    juce::ToggleButton formantPreserveButton { "Preserve" };

    std::unique_ptr<SliderAttachment> retuneSpeedAttach, humanizeAttach;
    std::unique_ptr<SliderAttachment> vibratoRateAttach, vibratoDepthAttach;
    std::unique_ptr<SliderAttachment> formantShiftAttach, bendRangeAttach;
    std::unique_ptr<ButtonAttachment> formantPreserveAttach;

    juce::Label retuneLabel, humanizeLabel, vibRateLabel, vibDepthLabel;
    juce::Label formantLabel, bendRangeLabel;

    void setupKnob(juce::Slider& slider, const juce::String& label = "");
};

class DynamicsTabContent : public juce::Component
{
public:
    DynamicsTabContent(ProVocalProcessor& p);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::Slider inputGainKnob;
    juce::Slider gateThreshKnob, gateRatioKnob, gateAttackKnob, gateReleaseKnob;
    juce::Slider hpfFreqKnob;
    juce::Slider compThreshKnob, compRatioKnob, compAttackKnob, compReleaseKnob, compMakeupKnob;
    juce::Slider deEssThreshKnob, deEssFreqKnob;

    std::unique_ptr<SliderAttachment> inputGainAttach;
    std::unique_ptr<SliderAttachment> gateThreshAttach, gateRatioAttach, gateAttackAttach, gateReleaseAttach;
    std::unique_ptr<SliderAttachment> hpfFreqAttach;
    std::unique_ptr<SliderAttachment> compThreshAttach, compRatioAttach, compAttackAttach, compReleaseAttach, compMakeupAttach;
    std::unique_ptr<SliderAttachment> deEssThreshAttach, deEssFreqAttach;

    juce::Label inputLabel, gateLabel, hpfLabel, compLabel, deEssLabel;

    void setupKnob(juce::Slider& slider);
};

class ToneTabContent : public juce::Component
{
public:
    ToneTabContent(ProVocalProcessor& p);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::Slider eqLowGainKnob, eqMidGainKnob, eqMidFreqKnob;
    juce::Slider eqMid2GainKnob, eqMid2FreqKnob;
    juce::Slider eqHighGainKnob;
    juce::Slider satDriveKnob;

    std::unique_ptr<SliderAttachment> eqLowGainAttach, eqMidGainAttach, eqMidFreqAttach;
    std::unique_ptr<SliderAttachment> eqMid2GainAttach, eqMid2FreqAttach;
    std::unique_ptr<SliderAttachment> eqHighGainAttach;
    std::unique_ptr<SliderAttachment> satDriveAttach;

    juce::Label eqLabel, satLabel;

    void setupKnob(juce::Slider& slider);
};

class FXTabContent : public juce::Component
{
public:
    FXTabContent(ProVocalProcessor& p);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    juce::Slider doublerMixKnob, doublerDetuneKnob, doublerDelayKnob, doublerWidthKnob;
    juce::Slider delayMixKnob, delayTimeKnob, delayFeedbackKnob, delayFilterKnob;
    juce::ToggleButton delaySyncButton { "Sync" };
    juce::Slider delaySyncDivKnob;
    juce::Slider reverbMixKnob, reverbSizeKnob;

    std::unique_ptr<SliderAttachment> doublerMixAttach, doublerDetuneAttach, doublerDelayAttach, doublerWidthAttach;
    std::unique_ptr<SliderAttachment> delayMixAttach, delayTimeAttach, delayFeedbackAttach, delayFilterAttach;
    std::unique_ptr<ButtonAttachment> delaySyncAttach;
    std::unique_ptr<SliderAttachment> delaySyncDivAttach;
    std::unique_ptr<SliderAttachment> reverbMixAttach, reverbSizeAttach;

    juce::Label doublerLabel, delayLabel, reverbLabel;

    void setupKnob(juce::Slider& slider);
};

// ============================================================================
// Main Editor
// ============================================================================

class ProVocalEditor : public juce::AudioProcessorEditor
{
public:
    explicit ProVocalEditor(ProVocalProcessor&);
    ~ProVocalEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ProVocalProcessor& processorRef;
    ProVocalLookAndFeel customLookAndFeel;

    // Header
    PresetBrowser presetBrowser;

    // Tab system
    TabPanel tabPanel;
    PitchTabContent pitchTab;
    DynamicsTabContent dynamicsTab;
    ToneTabContent toneTab;
    FXTabContent fxTab;

    // Persistent output section
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    juce::Slider outputGainSlider;
    std::unique_ptr<SliderAttachment> outputGainAttach;
    juce::Label outputLabel;
    LevelMeter levelMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProVocalEditor)
};
