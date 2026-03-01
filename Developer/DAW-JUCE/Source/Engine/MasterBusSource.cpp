#include "MasterBusSource.h"
#include "TrackAudioSource.h"

MasterBusSource::MasterBusSource (Transport& t) : transport (t) {}

void MasterBusSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;
    mixBuffer.setSize (2, samplesPerBlockExpected);

    const juce::ScopedLock sl (sourceLock);
    for (auto* src : trackSources)
        src->prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MasterBusSource::releaseResources()
{
    const juce::ScopedLock sl (sourceLock);
    for (auto* src : trackSources)
        src->releaseResources();
}

void MasterBusSource::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    if (! transport.isPlaying())
    {
        metering.reset();
        return;
    }

    auto numSamples = bufferToFill.numSamples;
    auto position = transport.getPositionInSamples();

    bool anySolo = false;
    {
        const juce::ScopedLock sl (sourceLock);
        for (auto* src : trackSources)
        {
            if (src->isSolo())
            {
                anySolo = true;
                break;
            }
        }
    }

    {
        const juce::ScopedLock sl (sourceLock);
        for (auto* src : trackSources)
        {
            if (src->isMuted())
                continue;
            if (anySolo && ! src->isSolo())
                continue;

            mixBuffer.setSize (2, numSamples, false, false, true);
            mixBuffer.clear();

            juce::AudioSourceChannelInfo trackInfo (&mixBuffer, 0, numSamples);
            src->setPlaybackPosition (position);
            src->getNextAudioBlock (trackInfo);

            for (int ch = 0; ch < juce::jmin (2, bufferToFill.buffer->getNumChannels()); ++ch)
                bufferToFill.buffer->addFrom (ch, bufferToFill.startSample,
                                              mixBuffer, ch, 0, numSamples);
        }
    }

    // Apply master volume
    float vol = masterVolume.load();
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
        bufferToFill.buffer->applyGain (bufferToFill.startSample, numSamples, vol);

    // Update metering
    const float* channels[2] = {
        bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample),
        bufferToFill.buffer->getNumChannels() > 1
            ? bufferToFill.buffer->getReadPointer (1, bufferToFill.startSample)
            : nullptr
    };
    metering.update (channels, bufferToFill.buffer->getNumChannels(), numSamples);

    // Advance transport
    transport.advancePosition (numSamples);
}

void MasterBusSource::addTrackSource (TrackAudioSource* source)
{
    const juce::ScopedLock sl (sourceLock);
    trackSources.addIfNotAlreadyThere (source);

    if (currentSampleRate > 0)
        source->prepareToPlay (currentBlockSize, currentSampleRate);
}

void MasterBusSource::removeTrackSource (TrackAudioSource* source)
{
    const juce::ScopedLock sl (sourceLock);
    trackSources.removeFirstMatchingValue (source);
}

void MasterBusSource::clearTrackSources()
{
    const juce::ScopedLock sl (sourceLock);
    trackSources.clear();
}
