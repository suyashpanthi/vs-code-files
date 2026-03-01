#include "PluginProcessor.h"
#include "PluginEditor.h"

ProVocalProcessor::ProVocalProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

ProVocalProcessor::~ProVocalProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ProVocalProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === Input Gain ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::inputGain, 1), "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "dB"));

    // === Gate ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::gateThresh, 1), "Gate Threshold",
        juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f), -80.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::gateRatio, 1), "Gate Ratio",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f, 0.3f), 10.0f, ":1"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::gateAttack, 1), "Gate Attack",
        juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.4f), 1.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::gateRelease, 1), "Gate Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.4f), 100.0f, "ms"));

    // === HPF ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::hpfFreq, 1), "HPF Frequency",
        juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.5f), 80.0f, "Hz"));

    // === Pitch Correction ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParamIDs::pitchCorrectionOn, 1), "Pitch Correction", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParamIDs::pitchRootNote, 1), "Root Note", 0, 11, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParamIDs::pitchScale, 1), "Scale", 0, 8, 1));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::pitchRetuneSpeed, 1), "Retune Speed",
        juce::NormalisableRange<float>(0.0f, 400.0f, 1.0f, 0.5f), 50.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::pitchHumanize, 1), "Humanize",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::pitchVibratoRate, 1), "Vibrato Rate",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), 0.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::pitchVibratoDepth, 1), "Vibrato Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, "cents"));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParamIDs::pitchMidiMode, 1), "MIDI Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParamIDs::pitchBendRange, 1), "Pitch Bend Range", 1, 24, 2));

    // === Formant ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::formantShift, 1), "Formant Shift",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f, "st"));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParamIDs::formantPreserve, 1), "Formant Preserve", true));

    // === Compressor ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compThresh, 1), "Comp Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -18.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compRatio, 1), "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 4.0f, ":1"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compAttack, 1), "Comp Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.4f), 10.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compRelease, 1), "Comp Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.4f), 100.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compMakeup, 1), "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f, "dB"));

    // === 4-Band EQ ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::eqLowGain, 1), "EQ Low Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::eqMidGain, 1), "EQ Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 2.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::eqMidFreq, 1), "EQ Mid Freq",
        juce::NormalisableRange<float>(500.0f, 5000.0f, 1.0f, 0.5f), 2500.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::eqMid2Gain, 1), "EQ Mid2 Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::eqMid2Freq, 1), "EQ Mid2 Freq",
        juce::NormalisableRange<float>(1000.0f, 10000.0f, 1.0f, 0.5f), 4000.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::eqHighGain, 1), "EQ High Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.0f, "dB"));

    // === De-Esser ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::deEssThresh, 1), "De-Esser Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), -20.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::deEssFreq, 1), "De-Esser Frequency",
        juce::NormalisableRange<float>(3000.0f, 10000.0f, 1.0f, 0.5f), 6000.0f, "Hz"));

    // === Saturation ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::satDrive, 1), "Saturation Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 15.0f, "%"));

    // === Doubler ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::doublerMix, 1), "Doubler Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::doublerDetune, 1), "Doubler Detune",
        juce::NormalisableRange<float>(1.0f, 50.0f, 0.1f), 10.0f, "cents"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::doublerDelay, 1), "Doubler Delay",
        juce::NormalisableRange<float>(5.0f, 80.0f, 0.1f), 20.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::doublerWidth, 1), "Doubler Width",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f, "%"));

    // === Delay ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::delayMix, 1), "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::delayTime, 1), "Delay Time",
        juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.4f), 250.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::delayFeedback, 1), "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f), 30.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::delayFilterFreq, 1), "Delay Filter",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.3f), 8000.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParamIDs::delaySync, 1), "Delay Sync", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID(ParamIDs::delaySyncDiv, 1), "Delay Sync Div", 0, 5, 0));

    // === Reverb ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::reverbMix, 1), "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 10.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::reverbSize, 1), "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 40.0f, "%"));

    // === Output ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::outputGain, 1), "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f, "dB"));

    // === Note Bypass (12 chromatic notes) ===
    const char* noteBypassIDs[] = {
        ParamIDs::noteBypass0, ParamIDs::noteBypass1, ParamIDs::noteBypass2,
        ParamIDs::noteBypass3, ParamIDs::noteBypass4, ParamIDs::noteBypass5,
        ParamIDs::noteBypass6, ParamIDs::noteBypass7, ParamIDs::noteBypass8,
        ParamIDs::noteBypass9, ParamIDs::noteBypass10, ParamIDs::noteBypass11
    };
    const char* noteNames[] = { "C Bypass", "C# Bypass", "D Bypass", "D# Bypass",
                                 "E Bypass", "F Bypass", "F# Bypass", "G Bypass",
                                 "G# Bypass", "A Bypass", "A# Bypass", "B Bypass" };
    for (int i = 0; i < 12; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(noteBypassIDs[i], 1), noteNames[i], false));
    }

    return { params.begin(), params.end() };
}

bool ProVocalProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void ProVocalProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    int numCh = getTotalNumOutputChannels();

    // [2] Gate
    gate.prepare(sampleRate, samplesPerBlock, numCh);

    // [3] HPF
    hpFilter.reset();
    hpFilter.prepare(spec);
    *hpFilter.state = *FilterCoeffs::makeHighPass(sampleRate, 80.0f);

    // [4] Pitch Detector
    pitchDetector.prepare(sampleRate, samplesPerBlock);

    // [5] Pitch Correction / Shifting / Formant
    pitchCorrector.prepare(sampleRate);
    pitchShifter.prepare(sampleRate, samplesPerBlock);
    formantShifter.prepare(sampleRate, samplesPerBlock);
    midiPitchHandler.reset();

    // Mono buffer for pitch processing
    monoBuffer.resize(static_cast<size_t>(samplesPerBlock), 0.0f);

    // [6] Compressor
    compressor.reset();
    compressor.prepare(spec);

    // [7] 4-Band EQ
    eqLowShelf.reset();
    eqLowShelf.prepare(spec);
    eqMidPeak.reset();
    eqMidPeak.prepare(spec);
    eqMid2Peak.reset();
    eqMid2Peak.prepare(spec);
    eqHighShelf.reset();
    eqHighShelf.prepare(spec);

    *eqLowShelf.state = *FilterCoeffs::makeLowShelf(sampleRate, 200.0f, 0.707f, 1.0f);
    *eqMidPeak.state = *FilterCoeffs::makePeakFilter(sampleRate, 2500.0f, 1.0f, 1.0f);
    *eqMid2Peak.state = *FilterCoeffs::makePeakFilter(sampleRate, 4000.0f, 1.0f, 1.0f);
    *eqHighShelf.state = *FilterCoeffs::makeHighShelf(sampleRate, 8000.0f, 0.707f, 1.0f);

    // [8] De-esser
    deEssBandFilter.reset();
    deEssBandFilter.prepare(spec);
    deEssCompressor.reset();
    deEssCompressor.prepare(spec);
    *deEssBandFilter.state = *FilterCoeffs::makeBandPass(sampleRate, 6000.0f);

    // [9] Saturation
    saturator.reset();
    saturator.prepare(spec);
    saturator.functionToUse = [](float x) { return std::tanh(x); };

    // [10] Doubler
    doubler.prepare(sampleRate, samplesPerBlock);

    // [11] Delay
    syncedDelay.prepare(sampleRate, samplesPerBlock, numCh);

    // [12] Reverb
    reverb.reset();
    reverb.prepare(spec);

    // Report latency
    int latency = pitchDetector.getLatencySamples();
    setLatencySamples(latency);
}

void ProVocalProcessor::releaseResources() {}

void ProVocalProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // ─── Read all parameters ───
    float inGain        = apvts.getRawParameterValue(ParamIDs::inputGain)->load();
    float gateThreshVal = apvts.getRawParameterValue(ParamIDs::gateThresh)->load();
    float gateRatioVal  = apvts.getRawParameterValue(ParamIDs::gateRatio)->load();
    float gateAttackVal = apvts.getRawParameterValue(ParamIDs::gateAttack)->load();
    float gateReleaseVal= apvts.getRawParameterValue(ParamIDs::gateRelease)->load();
    float hpfFreq       = apvts.getRawParameterValue(ParamIDs::hpfFreq)->load();

    bool pitchOn        = apvts.getRawParameterValue(ParamIDs::pitchCorrectionOn)->load() > 0.5f;
    int rootNote        = static_cast<int>(apvts.getRawParameterValue(ParamIDs::pitchRootNote)->load());
    int scaleType       = static_cast<int>(apvts.getRawParameterValue(ParamIDs::pitchScale)->load());
    float retuneSpeed   = apvts.getRawParameterValue(ParamIDs::pitchRetuneSpeed)->load();
    float humanize      = apvts.getRawParameterValue(ParamIDs::pitchHumanize)->load();
    float vibratoRate   = apvts.getRawParameterValue(ParamIDs::pitchVibratoRate)->load();
    float vibratoDepth  = apvts.getRawParameterValue(ParamIDs::pitchVibratoDepth)->load();
    bool midiMode       = apvts.getRawParameterValue(ParamIDs::pitchMidiMode)->load() > 0.5f;
    int bendRange       = static_cast<int>(apvts.getRawParameterValue(ParamIDs::pitchBendRange)->load());

    float fmtShift      = apvts.getRawParameterValue(ParamIDs::formantShift)->load();
    bool fmtPreserve    = apvts.getRawParameterValue(ParamIDs::formantPreserve)->load() > 0.5f;

    float compThresh    = apvts.getRawParameterValue(ParamIDs::compThresh)->load();
    float compRatio     = apvts.getRawParameterValue(ParamIDs::compRatio)->load();
    float compAttack    = apvts.getRawParameterValue(ParamIDs::compAttack)->load();
    float compRelease   = apvts.getRawParameterValue(ParamIDs::compRelease)->load();
    float compMakeup    = apvts.getRawParameterValue(ParamIDs::compMakeup)->load();

    float eqLowGain     = apvts.getRawParameterValue(ParamIDs::eqLowGain)->load();
    float eqMidGain     = apvts.getRawParameterValue(ParamIDs::eqMidGain)->load();
    float eqMidFreq     = apvts.getRawParameterValue(ParamIDs::eqMidFreq)->load();
    float eqMid2Gain    = apvts.getRawParameterValue(ParamIDs::eqMid2Gain)->load();
    float eqMid2Freq    = apvts.getRawParameterValue(ParamIDs::eqMid2Freq)->load();
    float eqHighGain    = apvts.getRawParameterValue(ParamIDs::eqHighGain)->load();

    float deEssThresh   = apvts.getRawParameterValue(ParamIDs::deEssThresh)->load();
    float deEssFreq     = apvts.getRawParameterValue(ParamIDs::deEssFreq)->load();
    float satDrive      = apvts.getRawParameterValue(ParamIDs::satDrive)->load();

    float dblMix        = apvts.getRawParameterValue(ParamIDs::doublerMix)->load();
    float dblDetune     = apvts.getRawParameterValue(ParamIDs::doublerDetune)->load();
    float dblDelay      = apvts.getRawParameterValue(ParamIDs::doublerDelay)->load();
    float dblWidth      = apvts.getRawParameterValue(ParamIDs::doublerWidth)->load();

    float dlyMix        = apvts.getRawParameterValue(ParamIDs::delayMix)->load();
    float dlyTime       = apvts.getRawParameterValue(ParamIDs::delayTime)->load();
    float dlyFeedback   = apvts.getRawParameterValue(ParamIDs::delayFeedback)->load();
    float dlyFilter     = apvts.getRawParameterValue(ParamIDs::delayFilterFreq)->load();
    bool dlySync        = apvts.getRawParameterValue(ParamIDs::delaySync)->load() > 0.5f;
    int dlySyncDiv      = static_cast<int>(apvts.getRawParameterValue(ParamIDs::delaySyncDiv)->load());

    float reverbMix     = apvts.getRawParameterValue(ParamIDs::reverbMix)->load();
    float reverbSize    = apvts.getRawParameterValue(ParamIDs::reverbSize)->load();
    float outGain       = apvts.getRawParameterValue(ParamIDs::outputGain)->load();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // === [1] Input Gain ===
    if (inGain != 0.0f)
        buffer.applyGain(juce::Decibels::decibelsToGain(inGain));

    // === [2] Gate ===
    if (gateThreshVal > -79.0f)
    {
        gate.setThreshold(gateThreshVal);
        gate.setRatio(gateRatioVal);
        gate.setAttack(gateAttackVal);
        gate.setRelease(gateReleaseVal);
        gate.process(buffer);
    }

    // === [3] High-Pass Filter ===
    *hpFilter.state = *FilterCoeffs::makeHighPass(currentSampleRate, hpfFreq);
    hpFilter.process(context);

    // === [4] Pitch Detection (mono sidechain, non-destructive) ===
    // Sum to mono for analysis
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float sum = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                sum += buffer.getSample(ch, i);
            monoBuffer[static_cast<size_t>(i)] = sum / static_cast<float>(numChannels);
        }
        pitchDetector.processSamples(monoBuffer.data(), numSamples);
    }

    float detectedPitch = pitchDetector.getDetectedPitch();
    float pitchConfidence = pitchDetector.getConfidence();

    // === [5] Pitch Correction + PSOLA + Formant ===
    float pitchRatio = 1.0f;
    float correctedPitch = detectedPitch;

    if (pitchOn && detectedPitch > 0.0f && pitchConfidence > 0.3f)
    {
        // Handle MIDI input
        midiPitchHandler.processMidi(midiMessages);
        midiPitchHandler.setPitchBendRange(bendRange);

        // Configure corrector
        pitchCorrector.setScale(rootNote, scaleType);
        pitchCorrector.setRetuneSpeed(retuneSpeed);
        pitchCorrector.setHumanize(humanize / 100.0f);
        pitchCorrector.setVibrato(vibratoRate, vibratoDepth);

        // Set note bypass
        for (int n = 0; n < 12; ++n)
        {
            juce::String paramID = "noteBypass" + juce::String(n);
            bool bypassed = apvts.getRawParameterValue(paramID)->load() > 0.5f;
            pitchCorrector.setNoteBypass(n, bypassed);
        }

        if (midiMode && midiPitchHandler.hasActiveNote())
        {
            pitchCorrector.setMidiTarget(midiPitchHandler.getTargetMidiNote());
            pitchCorrector.setPitchBend(midiPitchHandler.getPitchBendSemitones());
        }
        else
        {
            pitchCorrector.setMidiTarget(-1.0f);
            pitchCorrector.setPitchBend(0.0f);
        }

        pitchRatio = pitchCorrector.computeRatio(detectedPitch);
        correctedPitch = pitchCorrector.getCorrectedPitch();

        // Apply PSOLA pitch shifting to mono buffer
        float periodSamples = static_cast<float>(currentSampleRate) / detectedPitch;

        // Process each channel
        for (int ch = 0; ch < numChannels; ++ch)
        {
            // Copy channel to mono for processing
            auto* channelData = buffer.getWritePointer(ch);

            // Create a temp copy for the shifter
            std::vector<float> tempBuf(channelData, channelData + numSamples);
            pitchShifter.process(tempBuf.data(), numSamples, pitchRatio, periodSamples);

            // Formant preservation/shifting
            if (fmtPreserve || std::abs(fmtShift) > 0.01f)
                formantShifter.process(tempBuf.data(), numSamples, pitchRatio, fmtShift, fmtPreserve);

            std::memcpy(channelData, tempBuf.data(), static_cast<size_t>(numSamples) * sizeof(float));
        }
    }

    // Send pitch data to ring buffer for GUI
    PitchDataPoint pitchPoint;
    pitchPoint.inputPitchHz = detectedPitch;
    pitchPoint.outputPitchHz = correctedPitch;
    pitchPoint.confidence = pitchConfidence;
    pitchRingBuffer.push(pitchPoint);

    // === [6] Compressor ===
    compressor.setThreshold(compThresh);
    compressor.setRatio(compRatio);
    compressor.setAttack(compAttack);
    compressor.setRelease(compRelease);
    compressor.process(context);

    if (compMakeup != 0.0f)
        buffer.applyGain(juce::Decibels::decibelsToGain(compMakeup));

    // === [7] 4-Band EQ ===
    float lowGainLinear = juce::Decibels::decibelsToGain(eqLowGain);
    float midGainLinear = juce::Decibels::decibelsToGain(eqMidGain);
    float mid2GainLinear = juce::Decibels::decibelsToGain(eqMid2Gain);
    float highGainLinear = juce::Decibels::decibelsToGain(eqHighGain);

    *eqLowShelf.state = *FilterCoeffs::makeLowShelf(currentSampleRate, 200.0f, 0.707f, lowGainLinear);
    *eqMidPeak.state = *FilterCoeffs::makePeakFilter(currentSampleRate, eqMidFreq, 1.0f, midGainLinear);
    *eqMid2Peak.state = *FilterCoeffs::makePeakFilter(currentSampleRate, eqMid2Freq, 1.0f, mid2GainLinear);
    *eqHighShelf.state = *FilterCoeffs::makeHighShelf(currentSampleRate, 8000.0f, 0.707f, highGainLinear);

    eqLowShelf.process(context);
    eqMidPeak.process(context);
    eqMid2Peak.process(context);
    eqHighShelf.process(context);

    // === [8] De-Esser ===
    {
        juce::AudioBuffer<float> sidechain(numChannels, numSamples);
        for (int ch = 0; ch < numChannels; ++ch)
            sidechain.copyFrom(ch, 0, buffer, ch, 0, numSamples);

        juce::dsp::AudioBlock<float> scBlock(sidechain);
        juce::dsp::ProcessContextReplacing<float> scContext(scBlock);

        *deEssBandFilter.state = *FilterCoeffs::makeBandPass(currentSampleRate, deEssFreq);
        deEssBandFilter.process(scContext);

        float deEssThreshLinear = juce::Decibels::decibelsToGain(deEssThresh);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float scEnergy = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                scEnergy += std::abs(sidechain.getSample(ch, sample));
            scEnergy /= static_cast<float>(numChannels);

            if (scEnergy > deEssThreshLinear)
            {
                float reduction = deEssThreshLinear / (scEnergy + 1e-6f);
                reduction = juce::jmax(reduction, 0.1f);
                for (int ch = 0; ch < numChannels; ++ch)
                    buffer.setSample(ch, sample, buffer.getSample(ch, sample) * reduction);
            }
        }
    }

    // === [9] Saturation ===
    if (satDrive > 0.0f)
    {
        float driveAmount = 1.0f + (satDrive / 100.0f) * 4.0f;
        buffer.applyGain(driveAmount);
        saturator.process(context);
        buffer.applyGain(1.0f / driveAmount);
    }

    // === [10] Doubler ===
    if (dblMix > 0.0f)
    {
        doubler.setMix(dblMix / 100.0f);
        doubler.setDetuneCents(dblDetune);
        doubler.setDelayMs(dblDelay);
        doubler.setWidth(dblWidth / 100.0f);
        doubler.process(buffer);
    }

    // === [11] Delay ===
    if (dlyMix > 0.0f)
    {
        syncedDelay.setMix(dlyMix / 100.0f);
        syncedDelay.setDelayTimeMs(dlyTime);
        syncedDelay.setFeedback(dlyFeedback / 100.0f);
        syncedDelay.setFilterFreq(dlyFilter);
        syncedDelay.setSyncEnabled(dlySync);
        syncedDelay.setSyncDivision(dlySyncDiv);

        // Get tempo from host
        if (auto* playHead = getPlayHead())
        {
            if (auto posInfo = playHead->getPosition())
            {
                if (auto bpm = posInfo->getBpm())
                    syncedDelay.setTempoBPM(*bpm);
            }
        }

        syncedDelay.process(buffer);
    }

    // === [12] Reverb ===
    {
        juce::dsp::Reverb::Parameters reverbParams;
        reverbParams.roomSize = reverbSize / 100.0f;
        reverbParams.damping = 0.5f;
        reverbParams.wetLevel = reverbMix / 100.0f;
        reverbParams.dryLevel = 1.0f - (reverbMix / 100.0f);
        reverbParams.width = 1.0f;
        reverb.setParameters(reverbParams);
        reverb.process(context);
    }

    // === [13] Output Gain ===
    float outputGainLinear = juce::Decibels::decibelsToGain(outGain);
    buffer.applyGain(outputGainLinear);

    // Update output level for metering
    float level = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        level = juce::jmax(level, buffer.getMagnitude(ch, 0, numSamples));
    outputLevel.store(level);
}

juce::AudioProcessorEditor* ProVocalProcessor::createEditor()
{
    return new ProVocalEditor(*this);
}

void ProVocalProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ProVocalProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProVocalProcessor();
}
