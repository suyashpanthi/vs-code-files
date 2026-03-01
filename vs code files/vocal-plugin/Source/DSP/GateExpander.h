#pragma once
#include <JuceHeader.h>

class GateExpander
{
public:
    GateExpander();

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void setThreshold(float dB);
    void setRatio(float ratio);
    void setAttack(float ms);
    void setRelease(float ms);

    void process(juce::AudioBuffer<float>& buffer);

private:
    juce::dsp::NoiseGate<float> gate;
    juce::dsp::ProcessSpec spec {};
};
