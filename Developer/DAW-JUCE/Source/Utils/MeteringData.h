#pragma once
#include <atomic>
#include <cmath>

struct MeteringData
{
    std::atomic<float> peakL  { 0.0f };
    std::atomic<float> peakR  { 0.0f };
    std::atomic<float> rmsL   { 0.0f };
    std::atomic<float> rmsR   { 0.0f };

    void reset()
    {
        peakL.store (0.0f);
        peakR.store (0.0f);
        rmsL.store (0.0f);
        rmsR.store (0.0f);
    }

    void update (const float* const* channelData, int numChannels, int numSamples)
    {
        float pL = 0.0f, pR = 0.0f;
        float sumL = 0.0f, sumR = 0.0f;

        if (numChannels > 0 && channelData[0] != nullptr)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float s = std::abs (channelData[0][i]);
                if (s > pL) pL = s;
                sumL += channelData[0][i] * channelData[0][i];
            }
        }

        if (numChannels > 1 && channelData[1] != nullptr)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float s = std::abs (channelData[1][i]);
                if (s > pR) pR = s;
                sumR += channelData[1][i] * channelData[1][i];
            }
        }
        else
        {
            pR = pL;
            sumR = sumL;
        }

        peakL.store (pL);
        peakR.store (pR);
        rmsL.store (std::sqrt (sumL / (float) juce::jmax (1, numSamples)));
        rmsR.store (std::sqrt (sumR / (float) juce::jmax (1, numSamples)));
    }
};
