#pragma once
#include "PluginProcessor.h"

class ProVocalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ProVocalLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
};

class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter(ProVocalProcessor& p);
    void paint(juce::Graphics&) override;
    void timerCallback() override;
private:
    ProVocalProcessor& processor;
    float level = 0.0f;
};

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

    // Helper to create and configure a knob
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::Slider hpfFreqKnob;
    juce::Slider compThreshKnob, compRatioKnob, compAttackKnob, compReleaseKnob, compMakeupKnob;
    juce::Slider eqLowGainKnob, eqMidGainKnob, eqMidFreqKnob, eqHighGainKnob;
    juce::Slider deEssThreshKnob, deEssFreqKnob;
    juce::Slider satDriveKnob;
    juce::Slider reverbMixKnob, reverbSizeKnob;
    juce::Slider outputGainSlider;

    std::unique_ptr<SliderAttachment> hpfFreqAttach;
    std::unique_ptr<SliderAttachment> compThreshAttach, compRatioAttach, compAttackAttach, compReleaseAttach, compMakeupAttach;
    std::unique_ptr<SliderAttachment> eqLowGainAttach, eqMidGainAttach, eqMidFreqAttach, eqHighGainAttach;
    std::unique_ptr<SliderAttachment> deEssThreshAttach, deEssFreqAttach;
    std::unique_ptr<SliderAttachment> satDriveAttach;
    std::unique_ptr<SliderAttachment> reverbMixAttach, reverbSizeAttach;
    std::unique_ptr<SliderAttachment> outputGainAttach;

    juce::Label hpfLabel, compLabel, eqLabel, deEssLabel, satLabel, reverbLabel, outputLabel;

    LevelMeter levelMeter;

    void setupKnob(juce::Slider& slider);
    void setupLabel(juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProVocalEditor)
};
