#include "PitchDisplayComponent.h"
#include <cmath>

using namespace ProVocalColours;

PitchDisplayComponent::PitchDisplayComponent(RingBuffer<PitchDataPoint, 4096>& rb)
    : ringBuffer(rb)
{
    startTimerHz(60); // 60fps display refresh
}

void PitchDisplayComponent::timerCallback()
{
    PitchDataPoint point;
    bool updated = false;

    // Drain all available points from the ring buffer
    while (ringBuffer.pop(point))
    {
        displayData.push_back(point);
        updated = true;
    }

    // Trim to max display points
    while (displayData.size() > maxDisplayPoints)
        displayData.pop_front();

    if (updated)
        repaint();
}

float PitchDisplayComponent::hzToY(float hz, float height) const
{
    if (hz <= 0.0f) return height;

    // Map log frequency to y position
    // Range: 50Hz (bottom) to 1500Hz (top) — covers vocal range
    float minLog = std::log2(50.0f);
    float maxLog = std::log2(1500.0f);
    float noteLog = std::log2(hz);

    float normalized = (noteLog - minLog) / (maxLog - minLog);
    return height * (1.0f - juce::jlimit(0.0f, 1.0f, normalized));
}

juce::String PitchDisplayComponent::hzToNoteName(float hz) const
{
    if (hz <= 0.0f) return "";
    float midi = 69.0f + 12.0f * std::log2(hz / 440.0f);
    int noteNum = static_cast<int>(std::round(midi)) % 12;
    const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = static_cast<int>(std::round(midi)) / 12 - 1;
    return juce::String(names[noteNum]) + juce::String(octave);
}

void PitchDisplayComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    // Background
    g.setColour(juce::Colour(0xff0d0d1a));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Grid lines for notes (C2, C3, C4, C5, C6)
    g.setColour(accent.withAlpha(0.3f));
    float gridFreqs[] = {65.41f, 130.81f, 261.63f, 523.25f, 1046.50f};
    const char* gridLabels[] = {"C2", "C3", "C4", "C5", "C6"};
    for (int i = 0; i < 5; ++i)
    {
        float y = hzToY(gridFreqs[i], h);
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
        g.setColour(dimText.withAlpha(0.5f));
        g.setFont(9.0f);
        g.drawText(gridLabels[i], 2, static_cast<int>(y) - 8, 25, 16,
                   juce::Justification::centredLeft);
        g.setColour(accent.withAlpha(0.3f));
    }

    if (displayData.empty()) return;

    // Draw input pitch line
    juce::Path inputPath;
    juce::Path correctedPath;
    bool inputStarted = false;
    bool correctedStarted = false;

    float xStep = w / static_cast<float>(maxDisplayPoints);

    for (size_t i = 0; i < displayData.size(); ++i)
    {
        float x = static_cast<float>(i) * xStep;
        const auto& pt = displayData[i];

        if (pt.inputPitchHz > 0.0f && pt.confidence > 0.3f)
        {
            float y = hzToY(pt.inputPitchHz, h);
            if (!inputStarted)
            {
                inputPath.startNewSubPath(x, y);
                inputStarted = true;
            }
            else
            {
                inputPath.lineTo(x, y);
            }
        }
        else if (inputStarted)
        {
            inputStarted = false;
        }

        if (pt.outputPitchHz > 0.0f && pt.confidence > 0.3f)
        {
            float y = hzToY(pt.outputPitchHz, h);
            if (!correctedStarted)
            {
                correctedPath.startNewSubPath(x, y);
                correctedStarted = true;
            }
            else
            {
                correctedPath.lineTo(x, y);
            }
        }
        else if (correctedStarted)
        {
            correctedStarted = false;
        }
    }

    // Draw input pitch (blue)
    g.setColour(pitchLine.withAlpha(0.6f));
    g.strokePath(inputPath, juce::PathStrokeType(1.5f));

    // Draw corrected pitch (orange)
    g.setColour(pitchCorrected);
    g.strokePath(correctedPath, juce::PathStrokeType(2.0f));

    // Current note display
    if (!displayData.empty())
    {
        const auto& last = displayData.back();
        if (last.outputPitchHz > 0.0f && last.confidence > 0.3f)
        {
            g.setColour(text);
            g.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
            auto noteName = hzToNoteName(last.outputPitchHz);
            g.drawText(noteName, static_cast<int>(w) - 60, 5, 55, 20,
                       juce::Justification::centredRight);

            g.setColour(dimText);
            g.setFont(10.0f);
            g.drawText(juce::String(last.outputPitchHz, 1) + " Hz",
                       static_cast<int>(w) - 80, 23, 75, 14,
                       juce::Justification::centredRight);
        }
    }
}
