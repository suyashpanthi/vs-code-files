#pragma once
#include <JuceHeader.h>
#include "../Engine/AudioEngine.h"
#include "../Engine/TrackAudioSource.h"
#include "../Model/Project.h"
#include "Toolbar/TransportBar.h"
#include "Tracks/TrackListPanel.h"
#include "Mixer/MixerPanel.h"
#include "../FileIO/ProjectSerializer.h"
#include "../FileIO/AudioImporter.h"
#include "../FileIO/BounceExporter.h"

class MainComponent : public juce::Component,
                      public juce::KeyListener,
                      public juce::FileDragAndDropTarget
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // KeyListener
    using juce::Component::keyPressed;
    bool keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent) override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    AudioEngine engine;
    std::unique_ptr<Project> project;

    TransportBar transportBar;
    std::unique_ptr<TrackListPanel> trackListPanel;
    std::unique_ptr<MixerPanel> mixerPanel;

    ProjectSerializer projectSerializer;
    AudioImporter audioImporter;
    BounceExporter bounceExporter;

    juce::OwnedArray<TrackAudioSource> trackAudioSources;
    bool mixerVisible = false;

    void rebuildAudioSources();
    void syncTrackParameters();
    void newProject();
    void saveProject();
    void openProject();
    void showAudioSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
