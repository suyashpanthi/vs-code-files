#pragma once
#include <JuceHeader.h>
#include "../../Engine/Transport.h"

class AudioEngine;

class TransportBar : public juce::Component,
                     public juce::Timer
{
public:
    explicit TransportBar (AudioEngine& engine);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    AudioEngine& engine;

    juce::TextButton playButton   { "Play" };
    juce::TextButton stopButton   { "Stop" };
    juce::TextButton pauseButton  { "Pause" };
    juce::TextButton recordButton { "Rec" };
    juce::TextButton loopButton   { "Loop" };

    juce::Label timeDisplay;
    juce::Label barsDisplay;

    juce::Label bpmLabel;
    juce::TextEditor bpmEditor;

    juce::Label timeSigLabel;

    void updateTimeDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportBar)
};
