#pragma once
#include <JuceHeader.h>
#include "../Model/Project.h"
#include "../Engine/AudioEngine.h"

class ProjectSerializer
{
public:
    ProjectSerializer (AudioEngine& engine);

    bool saveProject (const Project& project, const juce::File& file);
    std::unique_ptr<Project> loadProject (const juce::File& file);

    void saveDialog (const Project& project);
    void openDialog (std::function<void (std::unique_ptr<Project>)> onLoaded);

private:
    AudioEngine& engine;
};
