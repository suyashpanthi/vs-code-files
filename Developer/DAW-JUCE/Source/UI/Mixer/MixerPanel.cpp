#include "MixerPanel.h"
#include "../Colours.h"

MixerPanel::MixerPanel (AudioEngine& e, Project& p)
    : engine (e), project (p), masterStrip (engine.getMasterBus())
{
    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&stripContainer, false);
    viewport.setScrollBarsShown (false, true);

    addAndMakeVisible (masterStrip);

    rebuildChannelStrips();
}

void MixerPanel::paint (juce::Graphics& g)
{
    g.fillAll (DAWColours::darkBackground);

    // Top border
    g.setColour (DAWColours::border);
    g.drawLine (0.0f, 0.0f, (float) getWidth(), 0.0f, 1.0f);
}

void MixerPanel::resized()
{
    auto area = getLocalBounds().reduced (4);

    // Master strip on the right
    masterStrip.setBounds (area.removeFromRight (80).reduced (2));
    area.removeFromRight (4);

    // Channel strips viewport
    viewport.setBounds (area);

    int totalWidth = channelStrips.size() * stripWidth;
    stripContainer.setSize (juce::jmax (totalWidth, area.getWidth()), area.getHeight());

    int x = 0;
    for (auto* strip : channelStrips)
    {
        strip->setBounds (x, 0, stripWidth - 4, stripContainer.getHeight());
        x += stripWidth;
    }
}

void MixerPanel::rebuildChannelStrips()
{
    channelStrips.clear();

    for (int i = 0; i < project.getTracks().size(); ++i)
    {
        auto* track = project.getTracks()[i];
        auto* strip = new ChannelStrip (*track);

        if (i < trackSources.size())
            strip->setMeteringData (&trackSources[i]->getMeteringData());

        strip->onMuteChanged = [this] { if (onMixChanged) onMixChanged(); };
        strip->onSoloChanged = [this] { if (onMixChanged) onMixChanged(); };
        strip->onVolumeChanged = [this](float) { if (onMixChanged) onMixChanged(); };
        strip->onPanChanged = [this](float) { if (onMixChanged) onMixChanged(); };

        stripContainer.addAndMakeVisible (strip);
        channelStrips.add (strip);
    }

    resized();
}

void MixerPanel::setTrackSources (const juce::Array<TrackAudioSource*>& sources)
{
    trackSources = sources;
    rebuildChannelStrips();
}
