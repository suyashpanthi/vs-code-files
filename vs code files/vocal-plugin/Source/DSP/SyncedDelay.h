#pragma once
#include <JuceHeader.h>

class SyncedDelay
{
public:
    SyncedDelay();

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void setMix(float mix01);
    void setDelayTimeMs(float ms);
    void setFeedback(float fb01);
    void setFilterFreq(float freq);
    void setSyncEnabled(bool enabled);
    void setSyncDivision(int division);  // 0=1/4, 1=1/8, 2=1/8d, 3=1/16, 4=1/4t, 5=1/8t
    void setTempoBPM(double bpm);

    void process(juce::AudioBuffer<float>& buffer);

    // Sync division names for UI
    static const char* getSyncDivisionName(int index);

private:
    double sr = 44100.0;
    double tempoBPM = 120.0;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 441000 };

    using FilterCoeffs = juce::dsp::IIR::Coefficients<float>;
    using MonoFilter = juce::dsp::IIR::Filter<float>;
    juce::dsp::ProcessorDuplicator<MonoFilter, FilterCoeffs> feedbackFilter;

    float mix = 0.0f;
    float delayTimeMs = 250.0f;
    float feedback = 0.3f;
    float filterFreq = 8000.0f;
    bool syncEnabled = false;
    int syncDivision = 0;

    std::vector<float> feedbackBuffer;

    float getDelayTimeSamples() const;
};
