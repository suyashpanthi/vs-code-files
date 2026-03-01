#pragma once
#include <JuceHeader.h>
#include "../../Engine/Transport.h"

class PlayheadOverlay : public juce::Component,
                        public juce::Timer
{
public:
    PlayheadOverlay (Transport& transport, double sampleRate);

    void paint (juce::Graphics& g) override;
    void timerCallback() override;

    void setPixelsPerBeat (double ppb) { pixelsPerBeat = ppb; }
    void setScrollOffset (int offset) { scrollOffset = offset; }
    void setSampleRate (double sr) { currentSampleRate = sr; }

private:
    Transport& transport;
    double pixelsPerBeat = 40.0;
    int scrollOffset = 0;
    double currentSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayheadOverlay)
};
