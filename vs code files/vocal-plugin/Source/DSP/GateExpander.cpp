#include "GateExpander.h"

GateExpander::GateExpander() {}

void GateExpander::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
    spec.numChannels = static_cast<juce::uint32>(numChannels);

    gate.prepare(spec);
    gate.setThreshold(-40.0f);
    gate.setRatio(10.0f);
    gate.setAttack(1.0f);
    gate.setRelease(100.0f);
}

void GateExpander::reset()
{
    gate.reset();
}

void GateExpander::setThreshold(float dB)
{
    gate.setThreshold(dB);
}

void GateExpander::setRatio(float ratio)
{
    gate.setRatio(ratio);
}

void GateExpander::setAttack(float ms)
{
    gate.setAttack(ms);
}

void GateExpander::setRelease(float ms)
{
    gate.setRelease(ms);
}

void GateExpander::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    gate.process(context);
}
