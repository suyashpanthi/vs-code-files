#pragma once
#include <JuceHeader.h>
#include "../../Model/Track.h"

class TrackHeaderComponent : public juce::Component
{
public:
    TrackHeaderComponent (Track& track);

    void paint (juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onMuteChanged;
    std::function<void()> onSoloChanged;
    std::function<void (float)> onVolumeChanged;
    std::function<void (float)> onPanChanged;

private:
    Track& track;

    juce::Label nameLabel;
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::TextButton armButton  { "R" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeaderComponent)
};
