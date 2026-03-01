#include "TrackHeaderComponent.h"
#include "../Colours.h"

TrackHeaderComponent::TrackHeaderComponent (Track& t) : track (t)
{
    nameLabel.setText (track.getName(), juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, DAWColours::textPrimary);
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (13.0f)));
    nameLabel.setEditable (true);
    nameLabel.onTextChange = [this] { track.setName (nameLabel.getText()); };
    addAndMakeVisible (nameLabel);

    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setRange (0.0, 2.0, 0.01);
    volumeSlider.setValue (track.getVolume(), juce::dontSendNotification);
    volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.onValueChange = [this]
    {
        track.setVolume ((float) volumeSlider.getValue());
        if (onVolumeChanged) onVolumeChanged ((float) volumeSlider.getValue());
    };
    addAndMakeVisible (volumeSlider);

    panSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setRange (-1.0, 1.0, 0.01);
    panSlider.setValue (track.getPan(), juce::dontSendNotification);
    panSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    panSlider.onValueChange = [this]
    {
        track.setPan ((float) panSlider.getValue());
        if (onPanChanged) onPanChanged ((float) panSlider.getValue());
    };
    addAndMakeVisible (panSlider);

    muteButton.setClickingTogglesState (true);
    muteButton.setToggleState (track.isMuted(), juce::dontSendNotification);
    muteButton.setColour (juce::TextButton::buttonOnColourId, DAWColours::muteColour);
    muteButton.onClick = [this]
    {
        track.setMuted (muteButton.getToggleState());
        if (onMuteChanged) onMuteChanged();
    };
    addAndMakeVisible (muteButton);

    soloButton.setClickingTogglesState (true);
    soloButton.setToggleState (track.isSolo(), juce::dontSendNotification);
    soloButton.setColour (juce::TextButton::buttonOnColourId, DAWColours::soloColour);
    soloButton.onClick = [this]
    {
        track.setSolo (soloButton.getToggleState());
        if (onSoloChanged) onSoloChanged();
    };
    addAndMakeVisible (soloButton);

    armButton.setClickingTogglesState (true);
    armButton.setColour (juce::TextButton::buttonOnColourId, DAWColours::recordColour);
    armButton.onClick = [this] { track.setArmed (armButton.getToggleState()); };
    addAndMakeVisible (armButton);
}

void TrackHeaderComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (DAWColours::panelBackground);
    g.fillRect (bounds);

    // Track colour strip
    g.setColour (track.getColour());
    g.fillRect (0.0f, 0.0f, 4.0f, (float) getHeight());

    // Border
    g.setColour (DAWColours::border);
    g.drawRect (bounds, 0.5f);
}

void TrackHeaderComponent::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    area.removeFromLeft (4); // colour strip

    nameLabel.setBounds (area.removeFromTop (20));

    auto controlArea = area.reduced (0, 2);
    auto buttonRow = controlArea.removeFromBottom (22);

    muteButton.setBounds (buttonRow.removeFromLeft (24));
    buttonRow.removeFromLeft (2);
    soloButton.setBounds (buttonRow.removeFromLeft (24));
    buttonRow.removeFromLeft (2);
    armButton.setBounds (buttonRow.removeFromLeft (24));
    buttonRow.removeFromLeft (4);
    panSlider.setBounds (buttonRow.removeFromLeft (28).withHeight (28).withCentre (
        { buttonRow.getX() - 14, buttonRow.getCentreY() }));

    volumeSlider.setBounds (controlArea);
}
