#include "MainComponent.h"
#include "Colours.h"

MainComponent::MainComponent()
    : transportBar (engine),
      projectSerializer (engine),
      audioImporter (engine),
      bounceExporter (engine)
{
    engine.initialise();
    newProject();

    addAndMakeVisible (transportBar);

    addKeyListener (this);
    setWantsKeyboardFocus (true);

    setSize (1400, 900);
}

MainComponent::~MainComponent()
{
    removeKeyListener (this);

    engine.getMasterBus().clearTrackSources();
    trackAudioSources.clear();
    engine.shutdown();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (DAWColours::background);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // Transport bar at top
    transportBar.setBounds (area.removeFromTop (44));

    if (mixerVisible && mixerPanel != nullptr)
    {
        // Mixer at bottom
        mixerPanel->setBounds (area.removeFromBottom (250));
    }

    // Track list fills remaining
    if (trackListPanel != nullptr)
        trackListPanel->setBounds (area);
}

bool MainComponent::keyPressed (const juce::KeyPress& key, juce::Component*)
{
    auto& transport = engine.getTransport();

    if (key == juce::KeyPress::spaceKey)
    {
        transport.togglePlayStop();
        return true;
    }

    if (key == juce::KeyPress::returnKey)
    {
        transport.returnToStart();
        return true;
    }

    if (key.getTextCharacter() == 'x' || key.getTextCharacter() == 'X')
    {
        mixerVisible = !mixerVisible;
        if (mixerPanel != nullptr)
            mixerPanel->setVisible (mixerVisible);
        resized();
        return true;
    }

    // Cmd+S save
    if (key.getModifiers().isCommandDown() && key.getKeyCode() == 'S')
    {
        saveProject();
        return true;
    }

    // Cmd+O open
    if (key.getModifiers().isCommandDown() && key.getKeyCode() == 'O')
    {
        openProject();
        return true;
    }

    // Cmd+N new
    if (key.getModifiers().isCommandDown() && key.getKeyCode() == 'N')
    {
        newProject();
        return true;
    }

    // Cmd+E bounce/export
    if (key.getModifiers().isCommandDown() && key.getKeyCode() == 'E')
    {
        bounceExporter.bounceDialog (*project);
        return true;
    }

    // Cmd+, audio settings
    if (key.getModifiers().isCommandDown() && key.getTextCharacter() == ',')
    {
        showAudioSettings();
        return true;
    }

    // +/- zoom
    if (key.getTextCharacter() == '=' || key.getTextCharacter() == '+')
    {
        if (trackListPanel != nullptr)
            trackListPanel->setPixelsPerBeat (trackListPanel->getPixelsPerBeat() * 1.2);
        return true;
    }

    if (key.getTextCharacter() == '-')
    {
        if (trackListPanel != nullptr)
            trackListPanel->setPixelsPerBeat (trackListPanel->getPixelsPerBeat() / 1.2);
        return true;
    }

    return false;
}

bool MainComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
    {
        auto file = juce::File (f);
        if (file.hasFileExtension ("wav;aif;aiff;mp3;flac;ogg;dawproj"))
            return true;
    }
    return false;
}

void MainComponent::filesDropped (const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (auto& f : files)
    {
        auto file = juce::File (f);
        if (file.hasFileExtension ("dawproj"))
        {
            auto loaded = projectSerializer.loadProject (file);
            if (loaded != nullptr)
            {
                project = std::move (loaded);
                project->setProjectFile (file);
                rebuildAudioSources();
            }
        }
        else if (file.hasFileExtension ("wav;aif;aiff;mp3;flac;ogg"))
        {
            audioImporter.importFile (*project, file);
            rebuildAudioSources();
        }
    }
}

void MainComponent::rebuildAudioSources()
{
    // Clear existing sources
    engine.getMasterBus().clearTrackSources();
    trackAudioSources.clear();

    // Create new audio sources for each track
    for (auto* track : project->getTracks())
    {
        auto* source = new TrackAudioSource (engine.getBackgroundThread());
        source->setVolume (track->getVolume());
        source->setPan (track->getPan());
        source->setMuted (track->isMuted());
        source->setSolo (track->isSolo());

        for (auto* region : track->getRegions())
            source->addRegion (region);

        trackAudioSources.add (source);
        engine.getMasterBus().addTrackSource (source);
    }

    // Rebuild UI
    trackListPanel = std::make_unique<TrackListPanel> (engine, *project);
    trackListPanel->onTracksChanged = [this]
    {
        rebuildAudioSources();
    };
    addAndMakeVisible (*trackListPanel);

    // Track sources array for mixer metering
    juce::Array<TrackAudioSource*> sourcePtrs;
    for (auto* s : trackAudioSources)
        sourcePtrs.add (s);

    mixerPanel = std::make_unique<MixerPanel> (engine, *project);
    mixerPanel->setTrackSources (sourcePtrs);
    mixerPanel->onMixChanged = [this] { syncTrackParameters(); };
    mixerPanel->setVisible (mixerVisible);
    addAndMakeVisible (*mixerPanel);

    resized();
}

void MainComponent::syncTrackParameters()
{
    for (int i = 0; i < project->getTracks().size() && i < trackAudioSources.size(); ++i)
    {
        auto* track = project->getTracks()[i];
        auto* source = trackAudioSources[i];

        source->setVolume (track->getVolume());
        source->setPan (track->getPan());
        source->setMuted (track->isMuted());
        source->setSolo (track->isSolo());
    }
}

void MainComponent::newProject()
{
    project = std::make_unique<Project>();
    rebuildAudioSources();
}

void MainComponent::saveProject()
{
    if (project->getProjectFile().existsAsFile())
    {
        projectSerializer.saveProject (*project, project->getProjectFile());
    }
    else
    {
        projectSerializer.saveDialog (*project);
    }
}

void MainComponent::openProject()
{
    projectSerializer.openDialog ([this](std::unique_ptr<Project> loaded)
    {
        if (loaded != nullptr)
        {
            project = std::move (loaded);
            rebuildAudioSources();
        }
    });
}

void MainComponent::showAudioSettings()
{
    auto* settingsComp = new juce::AudioDeviceSelectorComponent (
        engine.getDeviceManager(), 0, 2, 0, 2, true, true, true, false);
    settingsComp->setSize (500, 400);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned (settingsComp);
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = DAWColours::panelBackground;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.launchAsync();
}
