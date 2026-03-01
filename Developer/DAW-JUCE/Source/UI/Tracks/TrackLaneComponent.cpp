#include "TrackLaneComponent.h"
#include "../Colours.h"

TrackLaneComponent::TrackLaneComponent (Track& t, juce::AudioThumbnailCache& cache,
                                        juce::AudioFormatManager& fm)
    : track (t), thumbnailCache (cache), formatMgr (fm)
{
    rebuildWaveformViews();
}

void TrackLaneComponent::paint (juce::Graphics& g)
{
    g.fillAll (DAWColours::background.brighter (0.03f));

    // Draw grid lines (beats)
    g.setColour (DAWColours::border.withAlpha (0.3f));
    double beatWidth = pixelsPerBeat;
    int startBeat = (int) ((double) scrollOffset / beatWidth);
    int endBeat = startBeat + (int) ((double) getWidth() / beatWidth) + 2;

    for (int beat = juce::jmax (0, startBeat); beat <= endBeat; ++beat)
    {
        float x = (float) (beat * beatWidth - scrollOffset);
        g.drawLine (x, 0.0f, x, (float) getHeight(), 0.3f);
    }

    // Bottom border
    g.setColour (DAWColours::border);
    g.drawLine (0.0f, (float) getHeight() - 1, (float) getWidth(), (float) getHeight() - 1, 0.5f);
}

void TrackLaneComponent::resized()
{
    updateRegionBounds();
}

bool TrackLaneComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
    {
        auto file = juce::File (f);
        if (file.hasFileExtension ("wav;aif;aiff;mp3;flac;ogg"))
            return true;
    }
    return false;
}

void TrackLaneComponent::filesDropped (const juce::StringArray& files, int x, int /*y*/)
{
    for (auto& f : files)
    {
        auto file = juce::File (f);
        if (file.hasFileExtension ("wav;aif;aiff;mp3;flac;ogg"))
        {
            // Calculate position from x
            double beatAtX = (double) (x + scrollOffset) / pixelsPerBeat;
            double samplesPerBeatVal = sampleRate * 60.0 / bpm;
            int64_t posSamples = (int64_t) (beatAtX * samplesPerBeatVal);

            if (onFileDropped)
                onFileDropped (track, file, posSamples);
        }
    }

    rebuildWaveformViews();
}

void TrackLaneComponent::updateRegionBounds()
{
    double samplesPerBeatVal = sampleRate * 60.0 / bpm;

    for (int i = 0; i < waveformViews.size() && i < track.getRegions().size(); ++i)
    {
        auto* region = track.getRegions()[i];
        double startBeat = (double) region->getPositionInSamples() / samplesPerBeatVal;
        double lengthBeats = (double) region->getLengthInSamples() / samplesPerBeatVal;

        int xPos = (int) (startBeat * pixelsPerBeat - scrollOffset);
        int width = juce::jmax (10, (int) (lengthBeats * pixelsPerBeat));

        waveformViews[i]->setBounds (xPos, 2, width, getHeight() - 4);
    }
}

void TrackLaneComponent::rebuildWaveformViews()
{
    waveformViews.clear();

    for (auto* region : track.getRegions())
    {
        auto* wv = new WaveformView (thumbnailCache, formatMgr);
        wv->setRegion (region);
        wv->setColour (region->getColour());
        addAndMakeVisible (wv);
        waveformViews.add (wv);
    }

    updateRegionBounds();
}
