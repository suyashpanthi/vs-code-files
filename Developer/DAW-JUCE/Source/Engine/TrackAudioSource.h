#pragma once
#include <JuceHeader.h>
#include "../Utils/MeteringData.h"

class AudioRegion;

class TrackAudioSource : public juce::AudioSource
{
public:
    TrackAudioSource (juce::TimeSliceThread& backgroundThread);
    ~TrackAudioSource() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    void setPlaybackPosition (int64_t positionInSamples);

    // Region management
    void addRegion (AudioRegion* region);
    void removeRegion (AudioRegion* region);
    void clearRegions();

    // Mix parameters (lock-free)
    void setVolume (float vol) { volume.store (vol); }
    float getVolume() const    { return volume.load(); }

    void setPan (float p) { pan.store (juce::jlimit (-1.0f, 1.0f, p)); }
    float getPan() const  { return pan.load(); }

    void setMuted (bool m) { muted.store (m); }
    bool isMuted() const   { return muted.load(); }

    void setSolo (bool s) { solo.store (s); }
    bool isSolo() const   { return solo.load(); }

    MeteringData& getMeteringData() { return metering; }

private:
    juce::TimeSliceThread& bgThread;
    juce::OwnedArray<juce::BufferingAudioSource> bufferingSources;
    juce::Array<AudioRegion*> regions;
    juce::CriticalSection regionLock;

    std::atomic<float> volume { 1.0f };
    std::atomic<float> pan { 0.0f };
    std::atomic<bool> muted { false };
    std::atomic<bool> solo { false };

    int64_t currentPosition = 0;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    MeteringData metering;

    void applyPan (juce::AudioBuffer<float>& buffer, int numSamples);
};
