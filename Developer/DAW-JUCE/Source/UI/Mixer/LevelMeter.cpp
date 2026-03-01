#include "LevelMeter.h"
#include "../Colours.h"

LevelMeter::LevelMeter()
{
    startTimerHz (30);
}

void LevelMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (DAWColours::meterBackground);
    g.fillRoundedRectangle (bounds, 2.0f);

    float meterWidth = (bounds.getWidth() - 3.0f) / 2.0f;

    auto leftBounds = bounds.removeFromLeft (meterWidth).reduced (1.0f);
    bounds.removeFromLeft (1.0f);
    auto rightBounds = bounds.reduced (1.0f);

    drawMeterBar (g, leftBounds, displayPeakL);
    drawMeterBar (g, rightBounds, displayPeakR);
}

void LevelMeter::drawMeterBar (juce::Graphics& g, juce::Rectangle<float> bounds, float level)
{
    float clampedLevel = juce::jlimit (0.0f, 1.0f, level);
    float meterHeight = bounds.getHeight() * clampedLevel;
    auto meterBounds = bounds.removeFromBottom (meterHeight);

    // Gradient: green → yellow → red
    if (clampedLevel < 0.6f)
        g.setColour (DAWColours::meterGreen);
    else if (clampedLevel < 0.85f)
        g.setColour (DAWColours::meterYellow);
    else
        g.setColour (DAWColours::meterRed);

    g.fillRoundedRectangle (meterBounds, 1.0f);
}

void LevelMeter::timerCallback()
{
    if (meteringData != nullptr)
    {
        float newL = meteringData->peakL.load();
        float newR = meteringData->peakR.load();

        // Peak hold with decay
        displayPeakL = newL > displayPeakL ? newL : displayPeakL * decayRate;
        displayPeakR = newR > displayPeakR ? newR : displayPeakR * decayRate;
    }
    else
    {
        displayPeakL *= decayRate;
        displayPeakR *= decayRate;
    }

    repaint();
}
