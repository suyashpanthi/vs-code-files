#pragma once
#include <JuceHeader.h>
#include "../Utils/RingBuffer.h"
#include "ProVocalLookAndFeel.h"
#include <deque>

class PitchDisplayComponent : public juce::Component, private juce::Timer
{
public:
    PitchDisplayComponent(RingBuffer<PitchDataPoint, 4096>& pitchRingBuffer);

    void paint(juce::Graphics&) override;
    void resized() override {}
    void timerCallback() override;

private:
    RingBuffer<PitchDataPoint, 4096>& ringBuffer;

    static constexpr int maxDisplayPoints = 300;
    std::deque<PitchDataPoint> displayData;

    float hzToY(float hz, float height) const;
    juce::String hzToNoteName(float hz) const;
};
