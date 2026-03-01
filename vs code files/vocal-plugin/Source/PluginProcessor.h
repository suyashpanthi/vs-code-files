#pragma once
#include <JuceHeader.h>

class ProVocalProcessor : public juce::AudioProcessor
{
public:
    ProVocalProcessor();
    ~ProVocalProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    float getCurrentOutputLevel() const { return outputLevel.load(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // HPF
    using FilterCoeffs = juce::dsp::IIR::Coefficients<float>;
    using MonoFilter = juce::dsp::IIR::Filter<float>;
    using StereoFilter = juce::dsp::ProcessorDuplicator<MonoFilter, FilterCoeffs>;
    StereoFilter hpFilter;

    // Compressor
    juce::dsp::Compressor<float> compressor;

    // 3-Band EQ
    StereoFilter eqLowShelf;
    StereoFilter eqMidPeak;
    StereoFilter eqHighShelf;

    // De-esser (sidechain filtered compressor)
    StereoFilter deEssBandFilter;
    juce::dsp::Compressor<float> deEssCompressor;

    // Saturation
    juce::dsp::WaveShaper<float> saturator;

    // Reverb
    juce::dsp::Reverb reverb;

    // Output level for metering
    std::atomic<float> outputLevel { 0.0f };

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProVocalProcessor)
};
