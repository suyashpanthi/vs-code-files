#include "TrackListPanel.h"
#include "../Colours.h"

TrackListPanel::TrackListPanel (AudioEngine& e, Project& p)
    : engine (e), project (p),
      ruler (engine.getTransport()),
      playhead (engine.getTransport(), engine.getSampleRate())
{
    addAndMakeVisible (ruler);
    addAndMakeVisible (trackViewport);
    trackViewport.setViewedComponent (&trackContainer, false);
    trackViewport.setScrollBarsShown (true, true);
    addAndMakeVisible (playhead);

    rebuildTrackComponents();
}

void TrackListPanel::paint (juce::Graphics& g)
{
    g.fillAll (DAWColours::background);
}

void TrackListPanel::resized()
{
    auto area = getLocalBounds();

    // Ruler across top (after header width)
    auto rulerArea = area.removeFromTop (rulerHeight);
    rulerArea.removeFromLeft (headerWidth);
    ruler.setBounds (rulerArea);

    // Track viewport
    trackViewport.setBounds (area);

    int totalHeight = (int) project.getTracks().size() * trackHeight;
    int totalWidth = area.getWidth();
    trackContainer.setSize (totalWidth, juce::jmax (totalHeight, area.getHeight()));

    // Position headers and lanes
    int y = 0;
    for (int i = 0; i < headers.size(); ++i)
    {
        headers[i]->setBounds (0, y, headerWidth, trackHeight);
        lanes[i]->setBounds (headerWidth, y, totalWidth - headerWidth, trackHeight);
        y += trackHeight;
    }

    // Playhead overlay covers the lane area
    auto laneArea = area;
    laneArea.removeFromLeft (headerWidth);
    playhead.setBounds (laneArea.withY (0).withHeight (getHeight()));
}

void TrackListPanel::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (juce::ModifierKeys::currentModifiers.isCommandDown())
    {
        // Zoom
        double factor = wheel.deltaY > 0 ? 1.1 : 0.9;
        setPixelsPerBeat (pixelsPerBeat * factor);
    }
}

bool TrackListPanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
    {
        auto file = juce::File (f);
        if (file.hasFileExtension ("wav;aif;aiff;mp3;flac;ogg"))
            return true;
    }
    return false;
}

void TrackListPanel::filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (auto& f : files)
    {
        auto file = juce::File (f);
        if (! file.hasFileExtension ("wav;aif;aiff;mp3;flac;ogg"))
            continue;

        auto* track = project.addTrack (file.getFileNameWithoutExtension());
        if (track != nullptr)
        {
            auto* region = track->addRegion (file, 0, engine.getFormatManager());
            (void) region;
        }
    }

    rebuildTrackComponents();
    if (onTracksChanged) onTracksChanged();
}

void TrackListPanel::rebuildTrackComponents()
{
    // Remove old components
    headers.clear();
    lanes.clear();

    for (auto* track : project.getTracks())
    {
        auto* header = new TrackHeaderComponent (*track);
        header->onMuteChanged = [this] { if (onTracksChanged) onTracksChanged(); };
        header->onSoloChanged = [this] { if (onTracksChanged) onTracksChanged(); };
        header->onVolumeChanged = [this](float) { if (onTracksChanged) onTracksChanged(); };
        header->onPanChanged = [this](float) { if (onTracksChanged) onTracksChanged(); };
        trackContainer.addAndMakeVisible (header);
        headers.add (header);

        auto* lane = new TrackLaneComponent (*track, engine.getThumbnailCache(), engine.getFormatManager());
        lane->setSampleRate (engine.getSampleRate());
        lane->setBPM (engine.getTransport().getTempoMap().getBPM());
        lane->setPixelsPerBeat (pixelsPerBeat);
        lane->onFileDropped = [this](Track& t, const juce::File& f, int64_t pos) { handleFileDrop (t, f, pos); };
        trackContainer.addAndMakeVisible (lane);
        lanes.add (lane);
    }

    resized();
}

void TrackListPanel::setPixelsPerBeat (double ppb)
{
    pixelsPerBeat = juce::jlimit (5.0, 200.0, ppb);
    ruler.setPixelsPerBeat (pixelsPerBeat);
    playhead.setPixelsPerBeat (pixelsPerBeat);

    for (auto* lane : lanes)
        lane->setPixelsPerBeat (pixelsPerBeat);

    resized();
    repaint();
}

void TrackListPanel::handleFileDrop (Track& track, const juce::File& file, int64_t position)
{
    auto* region = track.addRegion (file, position, engine.getFormatManager());
    (void) region;
    rebuildTrackComponents();
    if (onTracksChanged) onTracksChanged();
}
