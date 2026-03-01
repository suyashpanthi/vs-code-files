#include "Doubler.h"
#include <cmath>

Doubler::Doubler() {}

void Doubler::prepare(double sampleRate, int maxBlockSize)
{
    sr = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = 1;

    delayL.prepare(spec);
    delayR.prepare(spec);
    delayL.setMaximumDelayInSamples(static_cast<int>(sampleRate)); // 1 sec max
    delayR.setMaximumDelayInSamples(static_cast<int>(sampleRate));

    reset();
}

void Doubler::reset()
{
    delayL.reset();
    delayR.reset();
    lfoPhaseL = 0.0f;
    lfoPhaseR = 0.5f;
}

void Doubler::setMix(float mix01)      { mix = juce::jlimit(0.0f, 1.0f, mix01); }
void Doubler::setDetuneCents(float c)  { detuneCents = c; }
void Doubler::setDelayMs(float ms)     { delayMs = ms; }
void Doubler::setWidth(float w)        { width = juce::jlimit(0.0f, 1.0f, w); }

void Doubler::process(juce::AudioBuffer<float>& buffer)
{
    if (mix <= 0.001f)
        return;

    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    float baseDelaySamples = delayMs * static_cast<float>(sr) / 1000.0f;
    // Detune creates pitch modulation via a slowly changing delay
    float detuneModDepth = detuneCents * 0.01f * static_cast<float>(sr) / 1000.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        // LFO for detune modulation
        float lfoL = std::sin(2.0f * juce::MathConstants<float>::pi * lfoPhaseL);
        float lfoR = std::sin(2.0f * juce::MathConstants<float>::pi * lfoPhaseR);

        float delayTimeSamplesL = baseDelaySamples + lfoL * detuneModDepth;
        float delayTimeSamplesR = baseDelaySamples + lfoR * detuneModDepth;

        delayTimeSamplesL = juce::jmax(1.0f, delayTimeSamplesL);
        delayTimeSamplesR = juce::jmax(1.0f, delayTimeSamplesR);

        // Get input (mono sum if stereo)
        float inputL = buffer.getSample(0, i);
        float inputR = (numChannels > 1) ? buffer.getSample(1, i) : inputL;

        // Push to delay lines
        delayL.pushSample(0, inputL);
        delayR.pushSample(0, inputR);

        // Get delayed+detuned signal
        float wetL = delayL.popSample(0, delayTimeSamplesL);
        float wetR = delayR.popSample(0, delayTimeSamplesR);

        // Apply width: at width=0, wet is mono; at width=1, fully stereo
        float wetMid = (wetL + wetR) * 0.5f;
        float wetSide = (wetL - wetR) * 0.5f;
        wetL = wetMid + wetSide * width;
        wetR = wetMid - wetSide * width;

        // Mix
        buffer.setSample(0, i, inputL * (1.0f - mix * 0.5f) + wetL * mix);
        if (numChannels > 1)
            buffer.setSample(1, i, inputR * (1.0f - mix * 0.5f) + wetR * mix);

        // Advance LFO
        float lfoInc = lfoRate / static_cast<float>(sr);
        lfoPhaseL += lfoInc;
        lfoPhaseR += lfoInc;
        if (lfoPhaseL >= 1.0f) lfoPhaseL -= 1.0f;
        if (lfoPhaseR >= 1.0f) lfoPhaseR -= 1.0f;
    }
}
