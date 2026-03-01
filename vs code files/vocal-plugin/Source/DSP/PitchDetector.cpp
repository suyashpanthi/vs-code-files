#include "PitchDetector.h"

PitchDetector::PitchDetector() {}

void PitchDetector::prepare(double sampleRate, int /*maxBlockSize*/)
{
    sr = sampleRate;
    windowSize = 2048;
    hopSize = 512;

    inputBuffer.resize(static_cast<size_t>(windowSize), 0.0f);
    yinBuffer.resize(static_cast<size_t>(windowSize / 2), 0.0f);
    inputWritePos = 0;

    detectedPitchHz = 0.0f;
    confidence = 0.0f;
}

void PitchDetector::reset()
{
    std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
    std::fill(yinBuffer.begin(), yinBuffer.end(), 0.0f);
    inputWritePos = 0;
    detectedPitchHz = 0.0f;
    confidence = 0.0f;
}

void PitchDetector::processSamples(const float* samples, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        inputBuffer[static_cast<size_t>(inputWritePos)] = samples[i];
        inputWritePos++;

        if (inputWritePos >= windowSize)
        {
            // Run YIN on the full window
            detectedPitchHz = yinDetectPitch(inputBuffer.data(), windowSize);

            // Shift buffer by hopSize: keep the last (windowSize - hopSize) samples
            int keep = windowSize - hopSize;
            std::memmove(inputBuffer.data(), inputBuffer.data() + hopSize,
                         static_cast<size_t>(keep) * sizeof(float));
            inputWritePos = keep;
        }
    }
}

float PitchDetector::yinDetectPitch(const float* buffer, int length)
{
    int halfLen = length / 2;

    // Step 1 & 2: Difference function
    yinDifference(buffer, length);

    // Step 3: Cumulative mean normalized difference
    yinCumulativeMeanNormalizedDifference();

    // Step 4: Absolute threshold
    int tauEstimate = yinAbsoluteThreshold();

    if (tauEstimate == -1)
    {
        confidence = 0.0f;
        return 0.0f;
    }

    // Step 5: Parabolic interpolation
    float betterTau = yinParabolicInterpolation(tauEstimate);

    float pitch = static_cast<float>(sr) / betterTau;

    // Validate range
    if (pitch < minFreq || pitch > maxFreq)
    {
        confidence = 0.0f;
        return 0.0f;
    }

    confidence = 1.0f - yinBuffer[static_cast<size_t>(tauEstimate)];
    return pitch;
}

void PitchDetector::yinDifference(const float* buffer, int length)
{
    int halfLen = length / 2;
    yinBuffer[0] = 0.0f;

    for (int tau = 1; tau < halfLen; ++tau)
    {
        float sum = 0.0f;
        for (int j = 0; j < halfLen; ++j)
        {
            float delta = buffer[j] - buffer[j + tau];
            sum += delta * delta;
        }
        yinBuffer[static_cast<size_t>(tau)] = sum;
    }
}

void PitchDetector::yinCumulativeMeanNormalizedDifference()
{
    yinBuffer[0] = 1.0f;
    float runningSum = 0.0f;

    for (size_t tau = 1; tau < yinBuffer.size(); ++tau)
    {
        runningSum += yinBuffer[tau];
        if (runningSum > 0.0f)
            yinBuffer[tau] = yinBuffer[tau] * static_cast<float>(tau) / runningSum;
        else
            yinBuffer[tau] = 1.0f;
    }
}

int PitchDetector::yinAbsoluteThreshold()
{
    int halfLen = static_cast<int>(yinBuffer.size());

    // Minimum tau based on max frequency
    int minTau = static_cast<int>(sr / maxFreq);
    // Maximum tau based on min frequency
    int maxTau = juce::jmin(static_cast<int>(sr / minFreq), halfLen - 1);

    for (int tau = minTau; tau < maxTau; ++tau)
    {
        if (yinBuffer[static_cast<size_t>(tau)] < yinThreshold)
        {
            // Find the local minimum
            while (tau + 1 < maxTau &&
                   yinBuffer[static_cast<size_t>(tau + 1)] < yinBuffer[static_cast<size_t>(tau)])
                ++tau;
            return tau;
        }
    }

    return -1; // no pitch detected
}

float PitchDetector::yinParabolicInterpolation(int tauEstimate)
{
    int x0 = tauEstimate - 1;
    int x2 = tauEstimate + 1;

    if (x0 < 0)
        return static_cast<float>(tauEstimate);

    if (x2 >= static_cast<int>(yinBuffer.size()))
        return static_cast<float>(tauEstimate);

    float s0 = yinBuffer[static_cast<size_t>(x0)];
    float s1 = yinBuffer[static_cast<size_t>(tauEstimate)];
    float s2 = yinBuffer[static_cast<size_t>(x2)];

    float denom = 2.0f * (2.0f * s1 - s2 - s0);
    if (std::abs(denom) < 1e-10f)
        return static_cast<float>(tauEstimate);

    float betterTau = static_cast<float>(tauEstimate) + (s2 - s0) / denom;
    return betterTau;
}
