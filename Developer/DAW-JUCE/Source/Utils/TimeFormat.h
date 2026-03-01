#pragma once
#include <JuceHeader.h>

namespace TimeFormat
{
    juce::String samplesToTimecode (int64_t samples, double sampleRate);
    juce::String samplesToBarsBeatsTicks (int64_t samples, double sampleRate, double bpm, int timeSigNum, int timeSigDen);
}
