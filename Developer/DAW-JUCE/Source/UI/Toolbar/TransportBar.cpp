#include "TransportBar.h"
#include "../../Engine/AudioEngine.h"
#include "../../Utils/TimeFormat.h"
#include "../Colours.h"

TransportBar::TransportBar (AudioEngine& e) : engine (e)
{
    // Play button
    playButton.setColour (juce::TextButton::buttonColourId, DAWColours::panelBackground);
    playButton.onClick = [this]
    {
        engine.getTransport().play();
    };
    addAndMakeVisible (playButton);

    // Stop button
    stopButton.setColour (juce::TextButton::buttonColourId, DAWColours::panelBackground);
    stopButton.onClick = [this]
    {
        engine.getTransport().stop();
    };
    addAndMakeVisible (stopButton);

    // Pause button
    pauseButton.setColour (juce::TextButton::buttonColourId, DAWColours::panelBackground);
    pauseButton.onClick = [this]
    {
        engine.getTransport().pause();
    };
    addAndMakeVisible (pauseButton);

    // Record button
    recordButton.setColour (juce::TextButton::buttonColourId, DAWColours::panelBackground);
    recordButton.setColour (juce::TextButton::textColourOffId, DAWColours::recordRed);
    addAndMakeVisible (recordButton);

    // Loop button
    loopButton.setColour (juce::TextButton::buttonColourId, DAWColours::panelBackground);
    loopButton.setClickingTogglesState (true);
    loopButton.onClick = [this]
    {
        engine.getTransport().setLoopEnabled (loopButton.getToggleState());
    };
    addAndMakeVisible (loopButton);

    // Time display
    timeDisplay.setFont (juce::Font (juce::FontOptions().withHeight (20.0f)));
    timeDisplay.setColour (juce::Label::textColourId, DAWColours::textPrimary);
    timeDisplay.setColour (juce::Label::backgroundColourId, DAWColours::darkBackground);
    timeDisplay.setJustificationType (juce::Justification::centred);
    timeDisplay.setText ("00:00:00.000", juce::dontSendNotification);
    addAndMakeVisible (timeDisplay);

    // Bars display
    barsDisplay.setFont (juce::Font (juce::FontOptions().withHeight (20.0f)));
    barsDisplay.setColour (juce::Label::textColourId, DAWColours::accent);
    barsDisplay.setColour (juce::Label::backgroundColourId, DAWColours::darkBackground);
    barsDisplay.setJustificationType (juce::Justification::centred);
    barsDisplay.setText ("1.1.000", juce::dontSendNotification);
    addAndMakeVisible (barsDisplay);

    // BPM
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setColour (juce::Label::textColourId, DAWColours::textSecondary);
    addAndMakeVisible (bpmLabel);

    bpmEditor.setText ("120");
    bpmEditor.setInputRestrictions (6, "0123456789.");
    bpmEditor.setJustification (juce::Justification::centred);
    bpmEditor.onReturnKey = [this]
    {
        double bpm = bpmEditor.getText().getDoubleValue();
        if (bpm >= 20.0 && bpm <= 999.0)
            engine.getTransport().getTempoMap().setBPM (bpm);
        bpmEditor.setText (juce::String (engine.getTransport().getTempoMap().getBPM(), 1));
    };
    addAndMakeVisible (bpmEditor);

    // Time sig
    timeSigLabel.setText ("4/4", juce::dontSendNotification);
    timeSigLabel.setColour (juce::Label::textColourId, DAWColours::textSecondary);
    timeSigLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (timeSigLabel);

    startTimerHz (30);
}

void TransportBar::paint (juce::Graphics& g)
{
    g.fillAll (DAWColours::headerBackground);
    g.setColour (DAWColours::border);
    g.drawLine (0.0f, (float) getHeight(), (float) getWidth(), (float) getHeight(), 1.0f);
}

void TransportBar::resized()
{
    auto area = getLocalBounds().reduced (8, 4);

    // Transport buttons
    auto buttonW = 55;
    auto buttonH = area.getHeight() - 4;

    stopButton.setBounds   (area.removeFromLeft (buttonW).withHeight (buttonH).withCentre ({area.getX() - buttonW / 2, area.getCentreY()}));
    area.removeFromLeft (4);
    playButton.setBounds   (area.removeFromLeft (buttonW).withHeight (buttonH));
    area.removeFromLeft (4);
    pauseButton.setBounds  (area.removeFromLeft (buttonW).withHeight (buttonH));
    area.removeFromLeft (4);
    recordButton.setBounds (area.removeFromLeft (buttonW).withHeight (buttonH));
    area.removeFromLeft (12);
    loopButton.setBounds   (area.removeFromLeft (buttonW).withHeight (buttonH));
    area.removeFromLeft (20);

    // Time displays
    timeDisplay.setBounds (area.removeFromLeft (140).withHeight (buttonH));
    area.removeFromLeft (8);
    barsDisplay.setBounds (area.removeFromLeft (100).withHeight (buttonH));
    area.removeFromLeft (20);

    // BPM
    bpmLabel.setBounds (area.removeFromLeft (30).withHeight (buttonH));
    bpmEditor.setBounds (area.removeFromLeft (60).withHeight (buttonH));
    area.removeFromLeft (12);

    // Time sig
    timeSigLabel.setBounds (area.removeFromLeft (40).withHeight (buttonH));
}

void TransportBar::timerCallback()
{
    updateTimeDisplay();

    // Update button colours based on transport state
    bool playing = engine.getTransport().isPlaying();
    playButton.setColour (juce::TextButton::textColourOffId,
                          playing ? DAWColours::playGreen : DAWColours::textPrimary);

    bool looping = engine.getTransport().isLoopEnabled();
    loopButton.setColour (juce::TextButton::textColourOffId,
                          looping ? DAWColours::accent : DAWColours::textPrimary);
    loopButton.setColour (juce::TextButton::textColourOnId, DAWColours::accent);
}

void TransportBar::updateTimeDisplay()
{
    auto& transport = engine.getTransport();
    auto pos = transport.getPositionInSamples();
    auto sr = engine.getSampleRate();
    auto bpm = transport.getTempoMap().getBPM();
    auto tsNum = transport.getTempoMap().getTimeSigNumerator();
    auto tsDen = transport.getTempoMap().getTimeSigDenominator();

    timeDisplay.setText (TimeFormat::samplesToTimecode (pos, sr), juce::dontSendNotification);
    barsDisplay.setText (TimeFormat::samplesToBarsBeatsTicks (pos, sr, bpm, tsNum, tsDen),
                         juce::dontSendNotification);
}
