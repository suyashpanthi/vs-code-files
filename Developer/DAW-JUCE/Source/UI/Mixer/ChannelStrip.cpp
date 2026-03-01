#include "ChannelStrip.h"
#include "../Colours.h"

ChannelStrip::ChannelStrip (Track& t) : track (t)
{
    nameLabel.setText (track.getName(), juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, DAWColours::textPrimary);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (11.0f)));
    addAndMakeVisible (nameLabel);

    faderSlider.setSliderStyle (juce::Slider::LinearVertical);
    faderSlider.setRange (0.0, 2.0, 0.01);
    faderSlider.setValue (track.getVolume(), juce::dontSendNotification);
    faderSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    faderSlider.onValueChange = [this]
    {
        track.setVolume ((float) faderSlider.getValue());
        if (onVolumeChanged) onVolumeChanged ((float) faderSlider.getValue());
    };
    addAndMakeVisible (faderSlider);

    panKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    panKnob.setRange (-1.0, 1.0, 0.01);
    panKnob.setValue (track.getPan(), juce::dontSendNotification);
    panKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    panKnob.onValueChange = [this]
    {
        track.setPan ((float) panKnob.getValue());
        if (onPanChanged) onPanChanged ((float) panKnob.getValue());
    };
    addAndMakeVisible (panKnob);

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

    addAndMakeVisible (meter);
}

void ChannelStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (DAWColours::panelBackground);
    g.fillRoundedRectangle (bounds, 3.0f);

    // Track colour strip at top
    g.setColour (track.getColour());
    g.fillRect (bounds.removeFromTop (3.0f));

    // Border
    g.setColour (DAWColours::border);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 3.0f, 0.5f);
}

void ChannelStrip::resized()
{
    auto area = getLocalBounds().reduced (4);
    area.removeFromTop (6); // colour strip space

    nameLabel.setBounds (area.removeFromTop (18));
    area.removeFromTop (4);

    panKnob.setBounds (area.removeFromTop (32).withSizeKeepingCentre (32, 32));
    area.removeFromTop (4);

    auto buttonRow = area.removeFromBottom (24);
    muteButton.setBounds (buttonRow.removeFromLeft (buttonRow.getWidth() / 2).reduced (1));
    soloButton.setBounds (buttonRow.reduced (1));

    area.removeFromBottom (4);

    // Fader and meter side by side
    auto meterArea = area.removeFromRight (14);
    meter.setBounds (meterArea);
    area.removeFromRight (2);
    faderSlider.setBounds (area);
}
