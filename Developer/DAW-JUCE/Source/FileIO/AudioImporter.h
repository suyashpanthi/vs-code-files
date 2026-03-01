#pragma once
#include <JuceHeader.h>

class Project;
class AudioEngine;

class AudioImporter
{
public:
    AudioImporter (AudioEngine& engine);

    void importFileDialog (Project& project);
    bool importFile (Project& project, const juce::File& file, int trackIndex = -1, int64_t position = 0);

private:
    AudioEngine& engine;
};
