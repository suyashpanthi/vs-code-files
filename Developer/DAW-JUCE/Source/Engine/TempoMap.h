#pragma once
#include <JuceHeader.h>

class TempoMap
{
public:
    TempoMap();

    void setBPM (double newBPM);
    double getBPM() const { return bpm.load(); }

    void setTimeSignature (int numerator, int denominator);
    int getTimeSigNumerator() const   { return timeSigNum.load(); }
    int getTimeSigDenominator() const { return timeSigDen.load(); }

    double samplesToBeats (int64_t samples, double sampleRate) const;
    int64_t beatsToSamples (double beats, double sampleRate) const;

    double samplesToSeconds (int64_t samples, double sampleRate) const;
    int64_t secondsToSamples (double seconds, double sampleRate) const;

    double beatsPerBar() const { return (double) timeSigNum.load(); }
    double samplesPerBeat (double sampleRate) const;
    double samplesPerBar (double sampleRate) const;

private:
    std::atomic<double> bpm { 120.0 };
    std::atomic<int> timeSigNum { 4 };
    std::atomic<int> timeSigDen { 4 };
};
