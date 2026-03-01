#pragma once
#include <JuceHeader.h>
#include <vector>

// Stereo doubler effect: creates a detuned, delayed copy for width
class Doubler
{
public:
    Doubler();

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void setMix(float mix01);           // 0-1
    void setDetuneCents(float cents);   // typically 5-30 cents
    void setDelayMs(float ms);          // typically 10-40 ms
    void setWidth(float width01);       // 0-1

    void process(juce::AudioBuffer<float>& buffer);

private:
    double sr = 44100.0;

    // Two delay lines for L and R
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL { 44100 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayR { 44100 };

    float mix = 0.0f;
    float detuneCents = 10.0f;
    float delayMs = 20.0f;
    float width = 1.0f;

    // Simple pitch shift via modulated delay
    float lfoPhaseL = 0.0f;
    float lfoPhaseR = 0.5f; // offset for stereo
    float lfoRate = 1.0f; // Hz
};
