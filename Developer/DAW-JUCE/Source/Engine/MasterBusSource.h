#pragma once
#include <JuceHeader.h>
#include "Transport.h"
#include "../Utils/MeteringData.h"

class TrackAudioSource;

class MasterBusSource : public juce::AudioSource
{
public:
    MasterBusSource (Transport& transport);

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    void addTrackSource (TrackAudioSource* source);
    void removeTrackSource (TrackAudioSource* source);
    void clearTrackSources();

    void setMasterVolume (float vol) { masterVolume.store (vol); }
    float getMasterVolume() const    { return masterVolume.load(); }

    MeteringData& getMeteringData() { return metering; }

private:
    Transport& transport;
    juce::Array<TrackAudioSource*> trackSources;
    juce::CriticalSection sourceLock;

    std::atomic<float> masterVolume { 1.0f };
    MeteringData metering;

    juce::AudioBuffer<float> mixBuffer;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
};
