#include "ProjectSerializer.h"

ProjectSerializer::ProjectSerializer (AudioEngine& e) : engine (e) {}

bool ProjectSerializer::saveProject (const Project& project, const juce::File& file)
{
    auto xml = project.toXml();
    if (xml == nullptr)
        return false;

    return xml->writeTo (file);
}

std::unique_ptr<Project> ProjectSerializer::loadProject (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("DAW_PROJECT"))
        return nullptr;

    return Project::fromXml (*xml, engine.getFormatManager());
}

void ProjectSerializer::saveDialog (const Project& project)
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Save Project",
        project.getProjectFile().existsAsFile() ? project.getProjectFile() : juce::File(),
        "*.dawproj");

    chooser->launchAsync (juce::FileBrowserComponent::saveMode,
        [this, &project, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
                saveProject (project, file.withFileExtension ("dawproj"));
        });
}

void ProjectSerializer::openDialog (std::function<void (std::unique_ptr<Project>)> onLoaded)
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Open Project",
        juce::File(),
        "*.dawproj");

    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, onLoaded, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                auto proj = loadProject (file);
                if (onLoaded && proj != nullptr)
                    onLoaded (std::move (proj));
            }
        });
}
