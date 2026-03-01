#pragma once
#include <JuceHeader.h>
#include "../../Utils/MeteringData.h"

class LevelMeter : public juce::Component,
                   public juce::Timer
{
public:
    LevelMeter();

    void setMeteringData (MeteringData* data) { meteringData = data; }
    void paint (juce::Graphics& g) override;
    void timerCallback() override;

private:
    MeteringData* meteringData = nullptr;
    float displayPeakL = 0.0f;
    float displayPeakR = 0.0f;
    float decayRate = 0.92f;

    void drawMeterBar (juce::Graphics& g, juce::Rectangle<float> bounds, float level);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
