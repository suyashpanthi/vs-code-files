#include "TimelineRuler.h"
#include "../Colours.h"
#include "../../Engine/TempoMap.h"

TimelineRuler::TimelineRuler (Transport& t) : transport (t) {}

void TimelineRuler::paint (juce::Graphics& g)
{
    g.fillAll (DAWColours::headerBackground);

    auto& tempo = transport.getTempoMap();
    int beatsPerBar = tempo.getTimeSigNumerator();
    auto bounds = getLocalBounds();

    g.setColour (DAWColours::textDim);
    g.setFont (10.0f);

    double beatWidth = pixelsPerBeat;
    int startBeat = (int) ((double) scrollOffset / beatWidth);
    int endBeat = startBeat + (int) ((double) bounds.getWidth() / beatWidth) + 2;

    for (int beat = juce::jmax (0, startBeat); beat <= endBeat; ++beat)
    {
        float x = (float) (beat * beatWidth - scrollOffset);

        if (beat % beatsPerBar == 0)
        {
            // Bar line
            g.setColour (DAWColours::textSecondary);
            g.drawLine (x, 0.0f, x, (float) bounds.getHeight(), 1.0f);

            int barNum = beat / beatsPerBar + 1;
            g.drawText (juce::String (barNum), (int) x + 3, 2, 40, 12,
                        juce::Justification::centredLeft);
        }
        else
        {
            // Beat tick
            g.setColour (DAWColours::border);
            g.drawLine (x, (float) bounds.getHeight() * 0.6f, x, (float) bounds.getHeight(), 0.5f);
        }
    }

    // Bottom border
    g.setColour (DAWColours::border);
    g.drawLine (0.0f, (float) bounds.getHeight() - 1, (float) bounds.getWidth(),
                (float) bounds.getHeight() - 1, 1.0f);
}

void TimelineRuler::mouseDown (const juce::MouseEvent& e)
{
    // Click on ruler to set playhead position
    double sr = 44100.0;
    auto& tempo = transport.getTempoMap();
    double beatAtClick = (double) (e.x + scrollOffset) / pixelsPerBeat;
    int64_t samplePos = tempo.beatsToSamples (beatAtClick, sr);
    transport.setPositionInSamples (samplePos);
}
