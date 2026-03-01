#pragma once
#include <JuceHeader.h>
#include "TrackHeaderComponent.h"
#include "TrackLaneComponent.h"
#include "TimelineRuler.h"
#include "PlayheadOverlay.h"
#include "../../Engine/AudioEngine.h"
#include "../../Model/Project.h"

class TrackListPanel : public juce::Component,
                       public juce::FileDragAndDropTarget
{
public:
    TrackListPanel (AudioEngine& engine, Project& project);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void rebuildTrackComponents();

    void setPixelsPerBeat (double ppb);
    double getPixelsPerBeat() const { return pixelsPerBeat; }

    std::function<void()> onTracksChanged;

private:
    AudioEngine& engine;
    Project& project;

    TimelineRuler ruler;
    PlayheadOverlay playhead;
    juce::Viewport trackViewport;
    juce::Component trackContainer;

    juce::OwnedArray<TrackHeaderComponent> headers;
    juce::OwnedArray<TrackLaneComponent> lanes;

    double pixelsPerBeat = 40.0;
    int scrollOffset = 0;

    static constexpr int headerWidth = 180;
    static constexpr int trackHeight = 80;
    static constexpr int rulerHeight = 24;

    void handleFileDrop (Track& track, const juce::File& file, int64_t position);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackListPanel)
};
