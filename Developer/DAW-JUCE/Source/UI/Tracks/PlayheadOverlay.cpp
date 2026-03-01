#include "PlayheadOverlay.h"
#include "../Colours.h"

PlayheadOverlay::PlayheadOverlay (Transport& t, double sampleRate)
    : transport (t), currentSampleRate (sampleRate)
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (30);
}

void PlayheadOverlay::paint (juce::Graphics& g)
{
    auto pos = transport.getPositionInSamples();
    auto& tempo = transport.getTempoMap();
    double beatPos = tempo.samplesToBeats (pos, currentSampleRate);
    float x = (float) (beatPos * pixelsPerBeat - scrollOffset);

    if (x >= 0.0f && x <= (float) getWidth())
    {
        g.setColour (DAWColours::playhead);
        g.drawLine (x, 0.0f, x, (float) getHeight(), 1.5f);

        // Small triangle at top
        juce::Path triangle;
        triangle.addTriangle (x - 5.0f, 0.0f, x + 5.0f, 0.0f, x, 8.0f);
        g.fillPath (triangle);
    }
}

void PlayheadOverlay::timerCallback()
{
    repaint();
}
