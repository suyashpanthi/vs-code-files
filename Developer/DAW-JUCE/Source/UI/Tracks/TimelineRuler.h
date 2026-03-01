#pragma once
#include <JuceHeader.h>
#include "../../Engine/Transport.h"

class TimelineRuler : public juce::Component
{
public:
    TimelineRuler (Transport& transport);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    void setPixelsPerBeat (double ppb) { pixelsPerBeat = ppb; repaint(); }
    double getPixelsPerBeat() const { return pixelsPerBeat; }

    void setScrollOffset (int offset) { scrollOffset = offset; repaint(); }

private:
    Transport& transport;
    double pixelsPerBeat = 40.0;
    int scrollOffset = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineRuler)
};
