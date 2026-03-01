#include "AudioImporter.h"
#include "../Engine/AudioEngine.h"
#include "../Model/Project.h"
#include "../UI/Colours.h"

AudioImporter::AudioImporter (AudioEngine& e) : engine (e) {}

void AudioImporter::importFileDialog (Project& project)
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Import Audio File",
        juce::File(),
        "*.wav;*.aif;*.aiff;*.mp3;*.flac;*.ogg");

    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, &project, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
                importFile (project, file);
        });
}

bool AudioImporter::importFile (Project& project, const juce::File& file, int trackIndex, int64_t position)
{
    if (! file.existsAsFile())
        return false;

    Track* track = nullptr;

    if (trackIndex >= 0 && trackIndex < project.getTracks().size())
    {
        track = project.getTracks()[trackIndex];
    }
    else
    {
        track = project.addTrack (file.getFileNameWithoutExtension());
    }

    if (track == nullptr)
        return false;

    auto* region = track->addRegion (file, position, engine.getFormatManager());
    return region != nullptr;
}
