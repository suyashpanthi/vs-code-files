#pragma once
#include <JuceHeader.h>
#include "Transport.h"
#include "MasterBusSource.h"

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    void initialise();
    void shutdown();

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager; }
    Transport& getTransport()                     { return transport; }
    MasterBusSource& getMasterBus()               { return masterBus; }
    juce::AudioFormatManager& getFormatManager()  { return formatManager; }
    juce::TimeSliceThread& getBackgroundThread()  { return backgroundThread; }
    juce::AudioThumbnailCache& getThumbnailCache() { return thumbnailCache; }

    double getSampleRate() const;

private:
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer sourcePlayer;
    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread backgroundThread { "DAW Background Thread" };

    Transport transport;
    MasterBusSource masterBus;

    juce::AudioThumbnailCache thumbnailCache { 32 };
};
