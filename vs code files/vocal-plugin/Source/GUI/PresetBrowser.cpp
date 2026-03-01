#include "PresetBrowser.h"

using namespace ProVocalColours;

PresetBrowser::PresetBrowser(juce::AudioProcessorValueTreeState& apvts)
    : apvtsRef(apvts)
{
    presetDirectory = getPresetDirectory();
    presetDirectory.createDirectory();

    presetCombo.setTextWhenNothingSelected("Select Preset...");
    presetCombo.onChange = [this] { loadSelectedPreset(); };
    addAndMakeVisible(presetCombo);

    saveButton.onClick = [this] { saveCurrentPreset(); };
    saveButton.setColour(juce::TextButton::buttonColourId, accent);
    saveButton.setColour(juce::TextButton::textColourOffId, text);
    addAndMakeVisible(saveButton);

    saveAsButton.onClick = [this] { savePresetAs(); };
    saveAsButton.setColour(juce::TextButton::buttonColourId, accent);
    saveAsButton.setColour(juce::TextButton::textColourOffId, text);
    addAndMakeVisible(saveAsButton);

    deleteButton.onClick = [this] { deleteSelectedPreset(); };
    deleteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff8b0000));
    deleteButton.setColour(juce::TextButton::textColourOffId, text);
    addAndMakeVisible(deleteButton);

    refreshPresetList();
}

void PresetBrowser::paint(juce::Graphics& /*g*/)
{
    // Transparent — drawn by parent
}

void PresetBrowser::resized()
{
    auto bounds = getLocalBounds();
    int buttonW = 60;
    int gap = 5;

    deleteButton.setBounds(bounds.removeFromRight(buttonW));
    bounds.removeFromRight(gap);
    saveAsButton.setBounds(bounds.removeFromRight(buttonW));
    bounds.removeFromRight(gap);
    saveButton.setBounds(bounds.removeFromRight(buttonW));
    bounds.removeFromRight(gap);
    presetCombo.setBounds(bounds);
}

juce::File PresetBrowser::getPresetDirectory() const
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("ProVocal")
        .getChildFile("Presets");
}

void PresetBrowser::refreshPresetList()
{
    presetCombo.clear();
    presetNames.clear();
    presetFiles.clear();

    auto files = presetDirectory.findChildFiles(juce::File::findFiles, false, "*.xml");
    files.sort();

    int id = 1;
    for (auto& file : files)
    {
        presetNames.add(file.getFileNameWithoutExtension());
        presetFiles.add(file);
        presetCombo.addItem(file.getFileNameWithoutExtension(), id++);
    }
}

void PresetBrowser::loadSelectedPreset()
{
    int index = presetCombo.getSelectedId() - 1;
    if (index >= 0 && index < presetFiles.size())
        loadPreset(presetFiles[index]);
}

void PresetBrowser::loadPreset(const juce::File& file)
{
    auto xml = juce::XmlDocument::parse(file);
    if (xml != nullptr && xml->hasTagName(apvtsRef.state.getType()))
        apvtsRef.replaceState(juce::ValueTree::fromXml(*xml));
}

void PresetBrowser::savePreset(const juce::File& file)
{
    auto state = apvtsRef.copyState();
    auto xml = state.createXml();
    if (xml != nullptr)
        xml->writeTo(file);
}

void PresetBrowser::saveCurrentPreset()
{
    int index = presetCombo.getSelectedId() - 1;
    if (index >= 0 && index < presetFiles.size())
    {
        savePreset(presetFiles[index]);
    }
    else
    {
        savePresetAs();
    }
}

void PresetBrowser::savePresetAs()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Save Preset", presetDirectory, "*.xml");

    chooser->launchAsync(juce::FileBrowserComponent::saveMode
                         | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                auto finalFile = file.withFileExtension("xml");
                savePreset(finalFile);
                refreshPresetList();

                // Select the newly saved preset
                for (int i = 0; i < presetFiles.size(); ++i)
                {
                    if (presetFiles[i] == finalFile)
                    {
                        presetCombo.setSelectedId(i + 1, juce::dontSendNotification);
                        break;
                    }
                }
            }
        });
}

void PresetBrowser::deleteSelectedPreset()
{
    int index = presetCombo.getSelectedId() - 1;
    if (index >= 0 && index < presetFiles.size())
    {
        presetFiles[index].deleteFile();
        refreshPresetList();
    }
}
