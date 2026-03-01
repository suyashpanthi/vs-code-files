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

    // HPF
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hpfFreq", 1), "HPF Frequency",
        juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.5f), 80.0f, "Hz"));

    // Compressor
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compThresh", 1), "Comp Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -18.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRatio", 1), "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f, 0.5f), 4.0f, ":1"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compAttack", 1), "Comp Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.4f), 10.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRelease", 1), "Comp Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.4f), 100.0f, "ms"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compMakeup", 1), "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f, "dB"));

    // EQ
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqLowGain", 1), "EQ Low Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqMidGain", 1), "EQ Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 2.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqMidFreq", 1), "EQ Mid Freq",
        juce::NormalisableRange<float>(500.0f, 5000.0f, 1.0f, 0.5f), 2500.0f, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("eqHighGain", 1), "EQ High Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.0f, "dB"));

    // De-Esser
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("deEssThresh", 1), "De-Esser Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), -20.0f, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("deEssFreq", 1), "De-Esser Frequency",
        juce::NormalisableRange<float>(3000.0f, 10000.0f, 1.0f, 0.5f), 6000.0f, "Hz"));

    // Saturation
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("satDrive", 1), "Saturation Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 15.0f, "%"));

    // Reverb
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbMix", 1), "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 10.0f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbSize", 1), "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 40.0f, "%"));

    // Output
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("outputGain", 1), "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f, "dB"));

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

    // HPF
    hpFilter.reset();
    hpFilter.prepare(spec);
    *hpFilter.state = *FilterCoeffs::makeHighPass(sampleRate, 80.0f);

    // Compressor
    compressor.reset();
    compressor.prepare(spec);

    // EQ
    eqLowShelf.reset();
    eqLowShelf.prepare(spec);
    eqMidPeak.reset();
    eqMidPeak.prepare(spec);
    eqHighShelf.reset();
    eqHighShelf.prepare(spec);

    *eqLowShelf.state = *FilterCoeffs::makeLowShelf(sampleRate, 200.0f, 0.707f, 1.0f);
    *eqMidPeak.state = *FilterCoeffs::makePeakFilter(sampleRate, 2500.0f, 1.0f, 1.0f);
    *eqHighShelf.state = *FilterCoeffs::makeHighShelf(sampleRate, 8000.0f, 0.707f, 1.0f);

    // De-esser
    deEssBandFilter.reset();
    deEssBandFilter.prepare(spec);
    deEssCompressor.reset();
    deEssCompressor.prepare(spec);
    *deEssBandFilter.state = *FilterCoeffs::makeBandPass(sampleRate, 6000.0f);

    // Saturation
    saturator.reset();
    saturator.prepare(spec);
    saturator.functionToUse = [](float x) { return std::tanh(x); };

    // Reverb
    reverb.reset();
    reverb.prepare(spec);
}

void ProVocalProcessor::releaseResources() {}

void ProVocalProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // Read parameters
    float hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
    float compThresh = apvts.getRawParameterValue("compThresh")->load();
    float compRatio = apvts.getRawParameterValue("compRatio")->load();
    float compAttack = apvts.getRawParameterValue("compAttack")->load();
    float compRelease = apvts.getRawParameterValue("compRelease")->load();
    float compMakeup = apvts.getRawParameterValue("compMakeup")->load();
    float eqLowGain = apvts.getRawParameterValue("eqLowGain")->load();
    float eqMidGain = apvts.getRawParameterValue("eqMidGain")->load();
    float eqMidFreq = apvts.getRawParameterValue("eqMidFreq")->load();
    float eqHighGain = apvts.getRawParameterValue("eqHighGain")->load();
    float deEssThresh = apvts.getRawParameterValue("deEssThresh")->load();
    float deEssFreq = apvts.getRawParameterValue("deEssFreq")->load();
    float satDrive = apvts.getRawParameterValue("satDrive")->load();
    float reverbMix = apvts.getRawParameterValue("reverbMix")->load();
    float reverbSize = apvts.getRawParameterValue("reverbSize")->load();
    float outGain = apvts.getRawParameterValue("outputGain")->load();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // === 1. High-Pass Filter ===
    *hpFilter.state = *FilterCoeffs::makeHighPass(currentSampleRate, hpfFreq);
    hpFilter.process(context);

    // === 2. Compressor ===
    compressor.setThreshold(compThresh);
    compressor.setRatio(compRatio);
    compressor.setAttack(compAttack);
    compressor.setRelease(compRelease);
    compressor.process(context);

    // Apply makeup gain
    if (compMakeup != 0.0f)
    {
        float makeupLinear = juce::Decibels::decibelsToGain(compMakeup);
        buffer.applyGain(makeupLinear);
    }

    // === 3. 3-Band EQ ===
    float lowGainLinear = juce::Decibels::decibelsToGain(eqLowGain);
    float midGainLinear = juce::Decibels::decibelsToGain(eqMidGain);
    float highGainLinear = juce::Decibels::decibelsToGain(eqHighGain);

    *eqLowShelf.state = *FilterCoeffs::makeLowShelf(currentSampleRate, 200.0f, 0.707f, lowGainLinear);
    *eqMidPeak.state = *FilterCoeffs::makePeakFilter(currentSampleRate, eqMidFreq, 1.0f, midGainLinear);
    *eqHighShelf.state = *FilterCoeffs::makeHighShelf(currentSampleRate, 8000.0f, 0.707f, highGainLinear);

    eqLowShelf.process(context);
    eqMidPeak.process(context);
    eqHighShelf.process(context);

    // === 4. De-Esser ===
    // Create a sidechain copy, bandpass it, then use envelope to duck the main signal
    {
        juce::AudioBuffer<float> sidechain(numChannels, numSamples);
        for (int ch = 0; ch < numChannels; ++ch)
            sidechain.copyFrom(ch, 0, buffer, ch, 0, numSamples);

        juce::dsp::AudioBlock<float> scBlock(sidechain);
        juce::dsp::ProcessContextReplacing<float> scContext(scBlock);

        *deEssBandFilter.state = *FilterCoeffs::makeBandPass(currentSampleRate, deEssFreq);
        deEssBandFilter.process(scContext);

        // Measure sidechain energy and apply gain reduction
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
                reduction = juce::jmax(reduction, 0.1f); // limit max reduction to -20dB
                for (int ch = 0; ch < numChannels; ++ch)
                    buffer.setSample(ch, sample, buffer.getSample(ch, sample) * reduction);
            }
        }
    }

    // === 5. Saturation ===
    if (satDrive > 0.0f)
    {
        float driveAmount = 1.0f + (satDrive / 100.0f) * 4.0f; // 1x to 5x drive
        buffer.applyGain(driveAmount);
        saturator.process(context);
        buffer.applyGain(1.0f / driveAmount); // compensate for drive level
    }

    // === 6. Plate Reverb ===
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

    // === 7. Output Gain ===
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
