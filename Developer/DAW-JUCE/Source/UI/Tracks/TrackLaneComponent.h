#pragma once
#include <JuceHeader.h>
#include "../../Model/Track.h"
#include "WaveformView.h"

class TrackLaneComponent : public juce::Component,
                           public juce::FileDragAndDropTarget
{
public:
    TrackLaneComponent (Track& track, juce::AudioThumbnailCache& cache,
                        juce::AudioFormatManager& formatManager);

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Drag-and-drop
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void setPixelsPerBeat (double ppb) { pixelsPerBeat = ppb; updateRegionBounds(); }
    void setScrollOffset (int offset) { scrollOffset = offset; updateRegionBounds(); }
    void setSampleRate (double sr) { sampleRate = sr; }
    void setBPM (double b) { bpm = b; updateRegionBounds(); }

    std::function<void (Track&, const juce::File&, int64_t)> onFileDropped;

private:
    Track& track;
    juce::AudioThumbnailCache& thumbnailCache;
    juce::AudioFormatManager& formatMgr;
    juce::OwnedArray<WaveformView> waveformViews;

    double pixelsPerBeat = 40.0;
    int scrollOffset = 0;
    double sampleRate = 44100.0;
    double bpm = 120.0;

    void updateRegionBounds();
    void rebuildWaveformViews();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackLaneComponent)
};
