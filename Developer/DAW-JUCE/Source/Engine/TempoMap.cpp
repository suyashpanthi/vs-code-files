#include "TempoMap.h"

TempoMap::TempoMap() {}

void TempoMap::setBPM (double newBPM)
{
    bpm.store (juce::jlimit (20.0, 999.0, newBPM));
}

void TempoMap::setTimeSignature (int numerator, int denominator)
{
    timeSigNum.store (juce::jlimit (1, 16, numerator));
    timeSigDen.store (juce::jlimit (1, 16, denominator));
}

double TempoMap::samplesToBeats (int64_t samples, double sampleRate) const
{
    if (sampleRate <= 0.0) return 0.0;
    return ((double) samples / sampleRate) * (bpm.load() / 60.0);
}

int64_t TempoMap::beatsToSamples (double beats, double sampleRate) const
{
    if (bpm.load() <= 0.0) return 0;
    return (int64_t) (beats * 60.0 / bpm.load() * sampleRate);
}

double TempoMap::samplesToSeconds (int64_t samples, double sampleRate) const
{
    if (sampleRate <= 0.0) return 0.0;
    return (double) samples / sampleRate;
}

int64_t TempoMap::secondsToSamples (double seconds, double sampleRate) const
{
    return (int64_t) (seconds * sampleRate);
}

double TempoMap::samplesPerBeat (double sampleRate) const
{
    return sampleRate * 60.0 / bpm.load();
}

double TempoMap::samplesPerBar (double sampleRate) const
{
    return samplesPerBeat (sampleRate) * beatsPerBar();
}
