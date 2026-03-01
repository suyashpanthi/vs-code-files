#include "LevelMeter.h"
#include "../PluginProcessor.h"
#include "ProVocalLookAndFeel.h"

using namespace ProVocalColours;

LevelMeter::LevelMeter(ProVocalProcessor& p) : processor(p)
{
    startTimerHz(30);
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    g.setColour(juce::Colour(0xff0d0d1a));
    g.fillRoundedRectangle(bounds, 3.0f);

    float meterHeight = bounds.getHeight() * juce::jmin(level, 1.0f);
    auto meterBounds = bounds.removeFromBottom(meterHeight);

    juce::Colour meterCol = meterGreen;
    if (level > 0.9f) meterCol = meterRed;
    else if (level > 0.7f) meterCol = meterYellow;

    g.setColour(meterCol);
    g.fillRoundedRectangle(meterBounds, 3.0f);
}

void LevelMeter::timerCallback()
{
    float newLevel = processor.getCurrentOutputLevel();
    level = level * 0.85f + newLevel * 0.15f;
    repaint();
}
