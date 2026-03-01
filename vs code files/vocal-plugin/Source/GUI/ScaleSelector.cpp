#include "ScaleSelector.h"

using namespace ProVocalColours;

ScaleSelector::ScaleSelector(juce::AudioProcessorValueTreeState& apvts)
{
    // Root note combo
    for (int i = 0; i < 12; ++i)
        rootNoteCombo.addItem(Scales::noteNames[i], i + 1);
    rootNoteCombo.setSelectedId(1); // C
    addAndMakeVisible(rootNoteCombo);

    rootLabel.setText("Key", juce::dontSendNotification);
    rootLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    rootLabel.setColour(juce::Label::textColourId, dimText);
    rootLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rootLabel);

    // Scale combo
    for (int i = 0; i < 9; ++i)
        scaleCombo.addItem(Scales::scaleNames[i], i + 1);
    scaleCombo.setSelectedId(2); // Major
    addAndMakeVisible(scaleCombo);

    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    scaleLabel.setColour(juce::Label::textColourId, dimText);
    scaleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(scaleLabel);

    // Combo attachments
    rootNoteAttach = std::make_unique<ComboAttachment>(apvts, "pitchRootNote", rootNoteCombo);
    scaleAttach = std::make_unique<ComboAttachment>(apvts, "pitchScale", scaleCombo);

    // Note bypass buttons
    for (int i = 0; i < 12; ++i)
    {
        noteButtons[static_cast<size_t>(i)].setButtonText(Scales::noteNames[i]);
        noteButtons[static_cast<size_t>(i)].setColour(juce::ToggleButton::textColourId, text);
        noteButtons[static_cast<size_t>(i)].setColour(juce::ToggleButton::tickColourId, knob);
        addAndMakeVisible(noteButtons[static_cast<size_t>(i)]);

        juce::String paramID = "noteBypass" + juce::String(i);
        noteAttachments[static_cast<size_t>(i)] =
            std::make_unique<ButtonAttachment>(apvts, paramID, noteButtons[static_cast<size_t>(i)]);
    }
}

void ScaleSelector::paint(juce::Graphics& g)
{
    g.setColour(sectionBg.withAlpha(0.5f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    g.setColour(knob);
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText("SCALE / KEY", 5, 2, 120, 18, juce::Justification::centredLeft);
}

void ScaleSelector::resized()
{
    int y = 22;

    rootLabel.setBounds(5, y, 30, 16);
    rootNoteCombo.setBounds(35, y, 60, 22);

    scaleLabel.setBounds(105, y, 35, 16);
    scaleCombo.setBounds(140, y, 120, 22);

    // Note bypass buttons in a row
    int noteY = y + 30;
    int noteW = getWidth() / 12;
    for (int i = 0; i < 12; ++i)
        noteButtons[static_cast<size_t>(i)].setBounds(i * noteW, noteY, noteW, 24);
}
