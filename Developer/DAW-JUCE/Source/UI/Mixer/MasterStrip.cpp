#include "MasterStrip.h"
#include "../Colours.h"

MasterStrip::MasterStrip (MasterBusSource& mb) : masterBus (mb)
{
    nameLabel.setText ("Master", juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, DAWColours::textPrimary);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (12.0f)));
    addAndMakeVisible (nameLabel);

    masterFader.setSliderStyle (juce::Slider::LinearVertical);
    masterFader.setRange (0.0, 2.0, 0.01);
    masterFader.setValue (masterBus.getMasterVolume(), juce::dontSendNotification);
    masterFader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    masterFader.onValueChange = [this]
    {
        masterBus.setMasterVolume ((float) masterFader.getValue());
    };
    addAndMakeVisible (masterFader);

    meter.setMeteringData (&masterBus.getMeteringData());
    addAndMakeVisible (meter);
}

void MasterStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (DAWColours::panelBackground.brighter (0.05f));
    g.fillRoundedRectangle (bounds, 3.0f);

    // Master label bar
    g.setColour (DAWColours::accent);
    g.fillRect (bounds.removeFromTop (3.0f));

    g.setColour (DAWColours::border);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 3.0f, 0.5f);
}

void MasterStrip::resized()
{
    auto area = getLocalBounds().reduced (4);
    area.removeFromTop (6);

    nameLabel.setBounds (area.removeFromTop (18));
    area.removeFromTop (4);

    auto meterArea = area.removeFromRight (18);
    meter.setBounds (meterArea);
    area.removeFromRight (2);
    masterFader.setBounds (area);
}
