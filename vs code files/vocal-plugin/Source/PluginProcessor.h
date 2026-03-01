#pragma once
#include <JuceHeader.h>
#include "Utils/RingBuffer.h"
#include "Utils/ParameterIDs.h"
#include "DSP/PitchDetector.h"
#include "DSP/PitchShifter.h"
#include "DSP/PitchCorrector.h"
#include "DSP/FormantShifter.h"
#include "DSP/GateExpander.h"
#include "DSP/Doubler.h"
#include "DSP/SyncedDelay.h"
#include "DSP/MidiPitchHandler.h"

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
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    float getCurrentOutputLevel() const { return outputLevel.load(); }

    // Pitch ring buffer for GUI visualization
    RingBuffer<PitchDataPoint, 4096> pitchRingBuffer;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Type aliases for filters
    using FilterCoeffs = juce::dsp::IIR::Coefficients<float>;
    using MonoFilter = juce::dsp::IIR::Filter<float>;
    using StereoFilter = juce::dsp::ProcessorDuplicator<MonoFilter, FilterCoeffs>;

    // === DSP Modules (processing order) ===

    // [1] Input Gain — applied directly

    // [2] Gate
    GateExpander gate;

    // [3] HPF
    StereoFilter hpFilter;

    // [4] Pitch Detection (mono, non-destructive)
    PitchDetector pitchDetector;

    // [5] Pitch Correction + PSOLA + Formant
    PitchCorrector pitchCorrector;
    PitchShifter pitchShifter;
    FormantShifter formantShifter;
    MidiPitchHandler midiPitchHandler;

    // [6] Compressor
    juce::dsp::Compressor<float> compressor;

    // [7] 4-Band EQ
    StereoFilter eqLowShelf;
    StereoFilter eqMidPeak;
    StereoFilter eqMid2Peak;
    StereoFilter eqHighShelf;

    // [8] De-esser
    StereoFilter deEssBandFilter;
    juce::dsp::Compressor<float> deEssCompressor;

    // [9] Saturation
    juce::dsp::WaveShaper<float> saturator;

    // [10] Doubler
    Doubler doubler;

    // [11] Delay
    SyncedDelay syncedDelay;

    // [12] Reverb
    juce::dsp::Reverb reverb;

    // [13] Output Gain — applied directly

    // Metering
    std::atomic<float> outputLevel { 0.0f };

    // Mono buffer for pitch processing
    std::vector<float> monoBuffer;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProVocalProcessor)
};
