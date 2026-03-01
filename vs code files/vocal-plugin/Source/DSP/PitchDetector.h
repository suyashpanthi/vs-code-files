#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

class PitchDetector
{
public:
    PitchDetector();

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // Feed audio samples. Call getDetectedPitch() after processing.
    void processSamples(const float* samples, int numSamples);

    float getDetectedPitch() const { return detectedPitchHz; }
    float getConfidence() const { return confidence; }

    // Latency in samples introduced by the detector
    int getLatencySamples() const { return windowSize; }

private:
    // YIN algorithm
    float yinDetectPitch(const float* buffer, int length);
    void yinDifference(const float* buffer, int length);
    void yinCumulativeMeanNormalizedDifference();
    int yinAbsoluteThreshold();
    float yinParabolicInterpolation(int tauEstimate);

    double sr = 44100.0;
    int windowSize = 2048;
    int hopSize = 512;

    std::vector<float> inputBuffer;
    int inputWritePos = 0;

    std::vector<float> yinBuffer;  // half window size

    float detectedPitchHz = 0.0f;
    float confidence = 0.0f;

    static constexpr float yinThreshold = 0.15f;
    static constexpr float minFreq = 50.0f;
    static constexpr float maxFreq = 1500.0f;
};
