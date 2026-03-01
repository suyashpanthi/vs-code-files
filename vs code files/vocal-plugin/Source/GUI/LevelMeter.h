#pragma once
#include <JuceHeader.h>

// Forward declare to avoid circular dependency
class ProVocalProcessor;

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
