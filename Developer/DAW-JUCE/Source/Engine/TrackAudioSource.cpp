#include "TrackAudioSource.h"
#include "../Model/AudioRegion.h"

TrackAudioSource::TrackAudioSource (juce::TimeSliceThread& backgroundThread)
    : bgThread (backgroundThread) {}

TrackAudioSource::~TrackAudioSource()
{
    releaseResources();
}

void TrackAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlockExpected;

    const juce::ScopedLock sl (regionLock);
    for (auto* bs : bufferingSources)
        bs->prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void TrackAudioSource::releaseResources()
{
    const juce::ScopedLock sl (regionLock);
    for (auto* bs : bufferingSources)
        bs->releaseResources();
}

void TrackAudioSource::setPlaybackPosition (int64_t positionInSamples)
{
    currentPosition = positionInSamples;
}

void TrackAudioSource::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    const juce::ScopedLock sl (regionLock);

    juce::AudioBuffer<float> regionBuffer (2, bufferToFill.numSamples);

    for (int i = 0; i < regions.size(); ++i)
    {
        auto* region = regions[i];
        auto regionStart = region->getPositionInSamples();
        auto regionEnd = regionStart + region->getLengthInSamples();

        // Check if region overlaps with current playback window
        auto blockEnd = currentPosition + bufferToFill.numSamples;
        if (currentPosition >= regionEnd || blockEnd <= regionStart)
            continue;

        // Calculate overlap
        auto overlapStart = juce::jmax (currentPosition, regionStart);
        auto overlapEnd = juce::jmin (blockEnd, regionEnd);
        auto overlapLen = (int) (overlapEnd - overlapStart);

        if (overlapLen <= 0)
            continue;

        auto bufferOffset = (int) (overlapStart - currentPosition);
        auto sourceOffset = region->getSourceOffsetInSamples() + (overlapStart - regionStart);

        // Position the buffering source and read
        if (i < bufferingSources.size())
        {
            bufferingSources[i]->setNextReadPosition (sourceOffset);
            regionBuffer.clear();
            juce::AudioSourceChannelInfo regionInfo (&regionBuffer, 0, overlapLen);
            bufferingSources[i]->getNextAudioBlock (regionInfo);

            float regionGain = region->getGain();
            for (int ch = 0; ch < juce::jmin (2, bufferToFill.buffer->getNumChannels()); ++ch)
                bufferToFill.buffer->addFrom (ch, bufferToFill.startSample + bufferOffset,
                                              regionBuffer, ch, 0, overlapLen, regionGain);
        }
    }

    // Apply volume
    float vol = volume.load();
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
        bufferToFill.buffer->applyGain (bufferToFill.startSample, bufferToFill.numSamples, vol);

    // Apply pan
    applyPan (*bufferToFill.buffer, bufferToFill.numSamples);

    // Update metering
    const float* channels[2] = {
        bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample),
        bufferToFill.buffer->getNumChannels() > 1
            ? bufferToFill.buffer->getReadPointer (1, bufferToFill.startSample)
            : nullptr
    };
    metering.update (channels, bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
}

void TrackAudioSource::addRegion (AudioRegion* region)
{
    const juce::ScopedLock sl (regionLock);
    regions.add (region);

    if (auto* reader = region->getFormatReader())
    {
        auto* readerSource = new juce::AudioFormatReaderSource (reader, false);
        auto* bufferingSource = new juce::BufferingAudioSource (readerSource, bgThread, false, 65536);
        bufferingSources.add (bufferingSource);

        if (currentSampleRate > 0)
            bufferingSource->prepareToPlay (currentBlockSize, currentSampleRate);
    }
}

void TrackAudioSource::removeRegion (AudioRegion* region)
{
    const juce::ScopedLock sl (regionLock);
    int idx = regions.indexOf (region);
    if (idx >= 0)
    {
        regions.remove (idx);
        if (idx < bufferingSources.size())
            bufferingSources.remove (idx);
    }
}

void TrackAudioSource::clearRegions()
{
    const juce::ScopedLock sl (regionLock);
    regions.clear();
    bufferingSources.clear();
}

void TrackAudioSource::applyPan (juce::AudioBuffer<float>& buffer, int numSamples)
{
    float p = pan.load();
    if (buffer.getNumChannels() < 2 || std::abs (p) < 0.001f)
        return;

    // Equal-power pan law
    float angle = (p + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
    float gainL = std::cos (angle);
    float gainR = std::sin (angle);

    buffer.applyGain (0, 0, numSamples, gainL);
    buffer.applyGain (1, 0, numSamples, gainR);
}
