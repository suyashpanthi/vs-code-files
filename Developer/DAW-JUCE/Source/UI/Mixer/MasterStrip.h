#pragma once
#include <JuceHeader.h>
#include "LevelMeter.h"
#include "../../Engine/MasterBusSource.h"

class MasterStrip : public juce::Component
{
public:
    MasterStrip (MasterBusSource& masterBus);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    MasterBusSource& masterBus;

    juce::Label nameLabel;
    juce::Slider masterFader;
    LevelMeter meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterStrip)
};
