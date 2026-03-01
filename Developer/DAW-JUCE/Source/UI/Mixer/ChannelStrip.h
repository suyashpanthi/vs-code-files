#pragma once
#include <JuceHeader.h>
#include "LevelMeter.h"
#include "../../Model/Track.h"

class ChannelStrip : public juce::Component
{
public:
    ChannelStrip (Track& track);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setMeteringData (MeteringData* data) { meter.setMeteringData (data); }

    std::function<void()> onMuteChanged;
    std::function<void()> onSoloChanged;
    std::function<void (float)> onVolumeChanged;
    std::function<void (float)> onPanChanged;

private:
    Track& track;

    juce::Label nameLabel;
    juce::Slider faderSlider;
    juce::Slider panKnob;
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    LevelMeter meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStrip)
};
