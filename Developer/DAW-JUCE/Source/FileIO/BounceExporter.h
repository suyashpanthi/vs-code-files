#pragma once
#include <JuceHeader.h>
#include "../Engine/AudioEngine.h"
#include "../Model/Project.h"

class BounceExporter
{
public:
    BounceExporter (AudioEngine& engine);

    void bounceDialog (Project& project);
    bool bounceToFile (Project& project, const juce::File& outputFile,
                       double sampleRate = 44100.0, int bitsPerSample = 24);

private:
    AudioEngine& engine;
};
