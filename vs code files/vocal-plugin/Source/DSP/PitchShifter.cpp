#include "PitchShifter.h"
#include <cmath>
#include <algorithm>

PitchShifter::PitchShifter() {}

void PitchShifter::prepare(double sampleRate, int maxBlockSize)
{
    sr = sampleRate;
    maxBlock = maxBlockSize;

    analysisBuffer.resize(bufferSize, 0.0f);
    synthesisBuffer.resize(bufferSize, 0.0f);
    outputBuffer.resize(bufferSize, 0.0f);

    reset();
}

void PitchShifter::reset()
{
    std::fill(analysisBuffer.begin(), analysisBuffer.end(), 0.0f);
    std::fill(synthesisBuffer.begin(), synthesisBuffer.end(), 0.0f);
    std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);

    analysisWritePos = 0;
    synthesisReadPos = 0;
    outputReadPos = 0;
    outputWritePos = 0;
    analysisPhase = 0.0f;
    synthesisPhase = 0.0f;
    prevPeriod = 200.0f;
    prevRatio = 1.0f;
}

int PitchShifter::getLatencySamples() const
{
    // ~1 period latency, roughly 10ms at typical vocal pitch
    return static_cast<int>(prevPeriod) + 256;
}

void PitchShifter::process(float* samples, int numSamples, float ratio, float detectedPeriodSamples)
{
    if (ratio <= 0.0f || ratio > 4.0f)
        ratio = 1.0f;

    // Clamp period to valid range
    float period = juce::jlimit(20.0f, 800.0f, detectedPeriodSamples);

    // Smooth period changes
    period = prevPeriod * 0.8f + period * 0.2f;
    prevPeriod = period;

    float synthPeriod = period / ratio;

    for (int i = 0; i < numSamples; ++i)
    {
        // Write input into analysis buffer
        analysisBuffer[static_cast<size_t>(analysisWritePos % bufferSize)] = samples[i];
        analysisWritePos++;

        // Advance synthesis phase
        synthesisPhase += 1.0f;

        if (synthesisPhase >= synthPeriod)
        {
            synthesisPhase -= synthPeriod;

            // Place a grain at the current analysis position
            float analysisCenterF = static_cast<float>(analysisWritePos) - period;
            int grainLength = static_cast<int>(period * 2.0f);

            // Overlap-add a Hann-windowed grain into the output buffer
            for (int g = 0; g < grainLength; ++g)
            {
                int analysisIdx = static_cast<int>(analysisCenterF) - grainLength / 2 + g;
                if (analysisIdx < 0) analysisIdx = 0;

                // Hann window
                float t = static_cast<float>(g) / static_cast<float>(grainLength);
                float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * t));

                float sampleVal = analysisBuffer[static_cast<size_t>(((analysisIdx % bufferSize) + bufferSize) % bufferSize)];
                int outIdx = (outputWritePos + g) % bufferSize;
                outputBuffer[static_cast<size_t>(outIdx)] += sampleVal * window;
            }

            outputWritePos = (outputWritePos + static_cast<int>(synthPeriod)) % bufferSize;
        }

        // Read from output buffer
        samples[i] = outputBuffer[static_cast<size_t>(outputReadPos % bufferSize)];
        outputBuffer[static_cast<size_t>(outputReadPos % bufferSize)] = 0.0f; // clear after read
        outputReadPos++;
    }
}
