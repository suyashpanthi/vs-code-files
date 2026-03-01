#pragma once
#include <JuceHeader.h>
#include "ChannelStrip.h"
#include "MasterStrip.h"
#include "../../Engine/AudioEngine.h"
#include "../../Model/Project.h"
#include "../../Engine/TrackAudioSource.h"

class MixerPanel : public juce::Component
{
public:
    MixerPanel (AudioEngine& engine, Project& project);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void rebuildChannelStrips();
    void setTrackSources (const juce::Array<TrackAudioSource*>& sources);

    std::function<void()> onMixChanged;

private:
    AudioEngine& engine;
    Project& project;

    juce::Viewport viewport;
    juce::Component stripContainer;
    juce::OwnedArray<ChannelStrip> channelStrips;
    MasterStrip masterStrip;

    juce::Array<TrackAudioSource*> trackSources;

    static constexpr int stripWidth = 70;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPanel)
};
