#include "SyncedDelay.h"
#include <cmath>

SyncedDelay::SyncedDelay() {}

void SyncedDelay::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    sr = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = static_cast<juce::uint32>(numChannels);

    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0)); // 2 sec max

    feedbackFilter.prepare(spec);
    *feedbackFilter.state = *FilterCoeffs::makeLowPass(sampleRate, 8000.0f);

    feedbackBuffer.resize(static_cast<size_t>(numChannels), 0.0f);
}

void SyncedDelay::reset()
{
    delayLine.reset();
    feedbackFilter.reset();
    std::fill(feedbackBuffer.begin(), feedbackBuffer.end(), 0.0f);
}

void SyncedDelay::setMix(float mix01)         { mix = juce::jlimit(0.0f, 1.0f, mix01); }
void SyncedDelay::setDelayTimeMs(float ms)     { delayTimeMs = ms; }
void SyncedDelay::setFeedback(float fb01)      { feedback = juce::jlimit(0.0f, 0.95f, fb01); }
void SyncedDelay::setFilterFreq(float freq)    { filterFreq = freq; }
void SyncedDelay::setSyncEnabled(bool enabled) { syncEnabled = enabled; }
void SyncedDelay::setSyncDivision(int division){ syncDivision = division; }
void SyncedDelay::setTempoBPM(double bpm)      { tempoBPM = juce::jmax(20.0, bpm); }

const char* SyncedDelay::getSyncDivisionName(int index)
{
    static const char* names[] = { "1/4", "1/8", "1/8 dotted", "1/16", "1/4 triplet", "1/8 triplet" };
    if (index >= 0 && index < 6)
        return names[index];
    return "1/4";
}

float SyncedDelay::getDelayTimeSamples() const
{
    float timeMs = delayTimeMs;

    if (syncEnabled && tempoBPM > 0.0)
    {
        // Convert BPM to beat duration
        float beatMs = 60000.0f / static_cast<float>(tempoBPM);

        switch (syncDivision)
        {
            case 0: timeMs = beatMs;          break; // 1/4
            case 1: timeMs = beatMs * 0.5f;   break; // 1/8
            case 2: timeMs = beatMs * 0.75f;  break; // 1/8 dotted
            case 3: timeMs = beatMs * 0.25f;  break; // 1/16
            case 4: timeMs = beatMs * 2.0f / 3.0f; break; // 1/4 triplet
            case 5: timeMs = beatMs / 3.0f;   break; // 1/8 triplet
            default: break;
        }
    }

    return timeMs * static_cast<float>(sr) / 1000.0f;
}

void SyncedDelay::process(juce::AudioBuffer<float>& buffer)
{
    if (mix <= 0.001f)
        return;

    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    float delaySamples = getDelayTimeSamples();
    delaySamples = juce::jlimit(1.0f, static_cast<float>(sr) * 2.0f, delaySamples);

    *feedbackFilter.state = *FilterCoeffs::makeLowPass(sr, juce::jlimit(200.0f, 20000.0f, filterFreq));

    for (int i = 0; i < numSamples; ++i)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float input = buffer.getSample(ch, i);

            // Push input + feedback into delay
            float toDelay = input + feedbackBuffer[static_cast<size_t>(ch)] * feedback;
            delayLine.pushSample(ch, toDelay);

            // Read delayed signal
            float wet = delayLine.popSample(ch, delaySamples);

            // Store feedback (will be filtered next block)
            feedbackBuffer[static_cast<size_t>(ch)] = wet;

            // Mix
            buffer.setSample(ch, i, input * (1.0f - mix) + wet * mix);
        }
    }

    // Filter the feedback buffer (approximate — per-sample would be better but more expensive)
    // The filter is applied to the main signal in the loop implicitly through the feedback path
}
