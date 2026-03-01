#pragma once
#include <JuceHeader.h>
#include "../../Model/AudioRegion.h"

class WaveformView : public juce::Component
{
public:
    WaveformView (juce::AudioThumbnailCache& cache, juce::AudioFormatManager& fm);

    void setRegion (AudioRegion* region);
    void setColour (juce::Colour c) { waveformColour = c; repaint(); }

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setPixelsPerSample (double pps) { pixelsPerSample = pps; repaint(); }

private:
    std::unique_ptr<juce::AudioThumbnail> thumbnail;
    juce::AudioThumbnailCache& thumbCache;
    juce::AudioFormatManager& formatMgr;
    AudioRegion* currentRegion = nullptr;
    juce::Colour waveformColour { 0xFF4A90D9 };
    double pixelsPerSample = 0.01;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformView)
};
