#pragma once
#include <JuceHeader.h>
#include <vector>

// TD-PSOLA (Time-Domain Pitch-Synchronous Overlap-Add) Pitch Shifter
class PitchShifter
{
public:
    PitchShifter();

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // Process a block of mono audio with a given pitch shift ratio
    // ratio > 1 = pitch up, ratio < 1 = pitch down
    // detectedPeriodSamples = T0 from pitch detector (sr / pitchHz)
    void process(float* samples, int numSamples, float ratio, float detectedPeriodSamples);

    int getLatencySamples() const;

private:
    double sr = 44100.0;
    int maxBlock = 512;

    // Analysis/synthesis buffers
    std::vector<float> analysisBuffer;
    std::vector<float> synthesisBuffer;
    int analysisWritePos = 0;
    int synthesisReadPos = 0;

    // Pitch marks
    float analysisPhase = 0.0f;
    float synthesisPhase = 0.0f;

    // Overlap-add output buffer
    std::vector<float> outputBuffer;
    int outputReadPos = 0;
    int outputWritePos = 0;

    // Internal state
    float prevPeriod = 200.0f;  // default ~220 Hz
    float prevRatio = 1.0f;

    static constexpr int bufferSize = 8192;

    void addGrain(float centerPos, float period, float* dest, int destLength);
};
