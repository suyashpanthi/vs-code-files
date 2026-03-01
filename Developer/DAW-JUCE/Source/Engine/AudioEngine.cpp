#include "AudioEngine.h"

AudioEngine::AudioEngine() : masterBus (transport) {}

AudioEngine::~AudioEngine()
{
    shutdown();
}

void AudioEngine::initialise()
{
    formatManager.registerBasicFormats();
    backgroundThread.startThread (juce::Thread::Priority::background);

    auto result = deviceManager.initialiseWithDefaultDevices (0, 2);
    if (result.isNotEmpty())
        DBG ("Audio device init error: " + result);

    sourcePlayer.setSource (&masterBus);
    deviceManager.addAudioCallback (&sourcePlayer);
}

void AudioEngine::shutdown()
{
    deviceManager.removeAudioCallback (&sourcePlayer);
    sourcePlayer.setSource (nullptr);
    backgroundThread.stopThread (2000);
}

double AudioEngine::getSampleRate() const
{
    if (auto* device = deviceManager.getCurrentAudioDevice())
        return device->getCurrentSampleRate();
    return 44100.0;
}
