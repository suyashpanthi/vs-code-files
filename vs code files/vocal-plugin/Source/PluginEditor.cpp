#include "PluginEditor.h"

using namespace ProVocalColours;

// ============================================================================
// Helper: setup a rotary knob
// ============================================================================
static void initKnob(juce::Slider& slider, juce::Component* parent)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 16);
    slider.setColour(juce::Slider::textBoxTextColourId, text);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    parent->addAndMakeVisible(slider);
}

static void initLabel(juce::Label& label, const juce::String& txt, juce::Component* parent)
{
    label.setText(txt, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, knob);
    label.setJustificationType(juce::Justification::centred);
    parent->addAndMakeVisible(label);
}

static void drawSectionBg(juce::Graphics& g, int x, int y, int w, int h, const juce::String& title)
{
    g.setColour(sectionBg.withAlpha(0.5f));
    g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y),
                           static_cast<float>(w), static_cast<float>(h), 6.0f);
    g.setColour(knob);
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText(title, x + 5, y + 3, w - 10, 16, juce::Justification::centredLeft);
}

// ============================================================================
// PITCH TAB
// ============================================================================
PitchTabContent::PitchTabContent(ProVocalProcessor& p)
    : pitchDisplay(p.pitchRingBuffer),
      scaleSelector(p.apvts)
{
    addAndMakeVisible(pitchDisplay);
    addAndMakeVisible(scaleSelector);

    pitchOnButton.setColour(juce::ToggleButton::textColourId, text);
    pitchOnButton.setColour(juce::ToggleButton::tickColourId, knob);
    addAndMakeVisible(pitchOnButton);
    pitchOnAttach = std::make_unique<ButtonAttachment>(p.apvts, ParamIDs::pitchCorrectionOn, pitchOnButton);

    midiModeButton.setColour(juce::ToggleButton::textColourId, text);
    midiModeButton.setColour(juce::ToggleButton::tickColourId, knob);
    addAndMakeVisible(midiModeButton);
    midiModeAttach = std::make_unique<ButtonAttachment>(p.apvts, ParamIDs::pitchMidiMode, midiModeButton);

    setupKnob(retuneSpeedKnob, "Retune");
    setupKnob(humanizeKnob, "Humanize");
    setupKnob(vibratoRateKnob, "Vib Rate");
    setupKnob(vibratoDepthKnob, "Vib Depth");
    setupKnob(formantShiftKnob, "Formant");
    setupKnob(bendRangeKnob, "Bend Rng");

    formantPreserveButton.setColour(juce::ToggleButton::textColourId, text);
    formantPreserveButton.setColour(juce::ToggleButton::tickColourId, knob);
    addAndMakeVisible(formantPreserveButton);
    formantPreserveAttach = std::make_unique<ButtonAttachment>(p.apvts, ParamIDs::formantPreserve, formantPreserveButton);

    retuneSpeedAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::pitchRetuneSpeed, retuneSpeedKnob);
    humanizeAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::pitchHumanize, humanizeKnob);
    vibratoRateAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::pitchVibratoRate, vibratoRateKnob);
    vibratoDepthAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::pitchVibratoDepth, vibratoDepthKnob);
    formantShiftAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::formantShift, formantShiftKnob);
    bendRangeAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::pitchBendRange, bendRangeKnob);
}

void PitchTabContent::setupKnob(juce::Slider& slider, const juce::String& /*label*/)
{
    initKnob(slider, this);
}

void PitchTabContent::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    drawSectionBg(g, 5, 310, getWidth() - 10, 85, "CORRECTION");
    drawSectionBg(g, 5, 400, 340, 85, "VIBRATO & FORMANT");
    drawSectionBg(g, 350, 400, 250, 85, "MIDI");
}

void PitchTabContent::resized()
{
    int knobSize = 60;

    // Pitch display at top
    pitchDisplay.setBounds(5, 5, getWidth() - 10, 220);

    // Scale selector below
    scaleSelector.setBounds(5, 230, getWidth() - 10, 75);

    // Correction section
    int corrY = 328;
    pitchOnButton.setBounds(10, corrY, 80, 22);
    retuneSpeedKnob.setBounds(100, corrY, knobSize, knobSize);
    humanizeKnob.setBounds(170, corrY, knobSize, knobSize);

    // Vibrato & Formant
    int vfY = 418;
    vibratoRateKnob.setBounds(10, vfY, knobSize, knobSize);
    vibratoDepthKnob.setBounds(80, vfY, knobSize, knobSize);
    formantShiftKnob.setBounds(160, vfY, knobSize, knobSize);
    formantPreserveButton.setBounds(230, vfY, 100, 22);

    // MIDI section
    int midiY = 418;
    midiModeButton.setBounds(355, midiY, 100, 22);
    bendRangeKnob.setBounds(460, midiY, knobSize, knobSize);
}

// ============================================================================
// DYNAMICS TAB
// ============================================================================
DynamicsTabContent::DynamicsTabContent(ProVocalProcessor& p)
{
    setupKnob(inputGainKnob);
    setupKnob(gateThreshKnob); setupKnob(gateRatioKnob);
    setupKnob(gateAttackKnob); setupKnob(gateReleaseKnob);
    setupKnob(hpfFreqKnob);
    setupKnob(compThreshKnob); setupKnob(compRatioKnob);
    setupKnob(compAttackKnob); setupKnob(compReleaseKnob); setupKnob(compMakeupKnob);
    setupKnob(deEssThreshKnob); setupKnob(deEssFreqKnob);

    inputGainAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::inputGain, inputGainKnob);
    gateThreshAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::gateThresh, gateThreshKnob);
    gateRatioAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::gateRatio, gateRatioKnob);
    gateAttackAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::gateAttack, gateAttackKnob);
    gateReleaseAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::gateRelease, gateReleaseKnob);
    hpfFreqAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::hpfFreq, hpfFreqKnob);
    compThreshAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::compThresh, compThreshKnob);
    compRatioAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::compRatio, compRatioKnob);
    compAttackAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::compAttack, compAttackKnob);
    compReleaseAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::compRelease, compReleaseKnob);
    compMakeupAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::compMakeup, compMakeupKnob);
    deEssThreshAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::deEssThresh, deEssThreshKnob);
    deEssFreqAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::deEssFreq, deEssFreqKnob);

    initLabel(inputLabel, "INPUT", this);
    initLabel(gateLabel, "GATE", this);
    initLabel(hpfLabel, "HPF", this);
    initLabel(compLabel, "COMPRESSOR", this);
    initLabel(deEssLabel, "DE-ESSER", this);
}

void DynamicsTabContent::setupKnob(juce::Slider& slider)
{
    initKnob(slider, this);
}

void DynamicsTabContent::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    drawSectionBg(g, 5, 5, 80, 240, "INPUT");
    drawSectionBg(g, 90, 5, 260, 240, "GATE");
    drawSectionBg(g, 355, 5, 80, 240, "HPF");
    drawSectionBg(g, 5, 255, 350, 240, "COMPRESSOR");
    drawSectionBg(g, 360, 255, 180, 240, "DE-ESSER");
}

void DynamicsTabContent::resized()
{
    int knobSize = 60;
    int row1Y = 30;
    int row2Y = row1Y + knobSize + 20;

    // Input section
    inputGainKnob.setBounds(15, row1Y, knobSize, knobSize);

    // Gate section
    gateThreshKnob.setBounds(100, row1Y, knobSize, knobSize);
    gateRatioKnob.setBounds(170, row1Y, knobSize, knobSize);
    gateAttackKnob.setBounds(240, row1Y, knobSize, knobSize);
    gateReleaseKnob.setBounds(100, row2Y, knobSize, knobSize);

    // HPF
    hpfFreqKnob.setBounds(365, row1Y, knobSize, knobSize);

    // Compressor section
    int compY = 280;
    compThreshKnob.setBounds(15, compY, knobSize, knobSize);
    compRatioKnob.setBounds(85, compY, knobSize, knobSize);
    compAttackKnob.setBounds(155, compY, knobSize, knobSize);
    compReleaseKnob.setBounds(225, compY, knobSize, knobSize);
    compMakeupKnob.setBounds(15, compY + knobSize + 15, knobSize, knobSize);

    // De-Esser section
    deEssThreshKnob.setBounds(370, compY, knobSize, knobSize);
    deEssFreqKnob.setBounds(370, compY + knobSize + 15, knobSize, knobSize);
}

// ============================================================================
// TONE TAB
// ============================================================================
ToneTabContent::ToneTabContent(ProVocalProcessor& p)
{
    setupKnob(eqLowGainKnob); setupKnob(eqMidGainKnob); setupKnob(eqMidFreqKnob);
    setupKnob(eqMid2GainKnob); setupKnob(eqMid2FreqKnob);
    setupKnob(eqHighGainKnob);
    setupKnob(satDriveKnob);

    eqLowGainAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::eqLowGain, eqLowGainKnob);
    eqMidGainAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::eqMidGain, eqMidGainKnob);
    eqMidFreqAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::eqMidFreq, eqMidFreqKnob);
    eqMid2GainAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::eqMid2Gain, eqMid2GainKnob);
    eqMid2FreqAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::eqMid2Freq, eqMid2FreqKnob);
    eqHighGainAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::eqHighGain, eqHighGainKnob);
    satDriveAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::satDrive, satDriveKnob);

    initLabel(eqLabel, "4-BAND EQ", this);
    initLabel(satLabel, "SATURATION", this);
}

void ToneTabContent::setupKnob(juce::Slider& slider)
{
    initKnob(slider, this);
}

void ToneTabContent::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    drawSectionBg(g, 5, 5, 500, 280, "4-BAND EQ");
    drawSectionBg(g, 5, 295, 120, 190, "SATURATION");
}

void ToneTabContent::resized()
{
    int knobSize = 65;
    int row1Y = 35;
    int row2Y = row1Y + knobSize + 30;

    // EQ row 1: Low Shelf, Mid1 Gain, Mid1 Freq
    eqLowGainKnob.setBounds(20, row1Y, knobSize, knobSize);
    eqMidGainKnob.setBounds(100, row1Y, knobSize, knobSize);
    eqMidFreqKnob.setBounds(180, row1Y, knobSize, knobSize);

    // EQ row 2: Mid2 Gain, Mid2 Freq, High Shelf
    eqMid2GainKnob.setBounds(20, row2Y, knobSize, knobSize);
    eqMid2FreqKnob.setBounds(100, row2Y, knobSize, knobSize);
    eqHighGainKnob.setBounds(180, row2Y, knobSize, knobSize);

    // Saturation
    satDriveKnob.setBounds(20, 325, knobSize, knobSize);
}

// ============================================================================
// FX TAB
// ============================================================================
FXTabContent::FXTabContent(ProVocalProcessor& p)
{
    setupKnob(doublerMixKnob); setupKnob(doublerDetuneKnob);
    setupKnob(doublerDelayKnob); setupKnob(doublerWidthKnob);
    setupKnob(delayMixKnob); setupKnob(delayTimeKnob);
    setupKnob(delayFeedbackKnob); setupKnob(delayFilterKnob);
    setupKnob(delaySyncDivKnob);
    setupKnob(reverbMixKnob); setupKnob(reverbSizeKnob);

    delaySyncButton.setColour(juce::ToggleButton::textColourId, text);
    delaySyncButton.setColour(juce::ToggleButton::tickColourId, knob);
    addAndMakeVisible(delaySyncButton);

    doublerMixAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::doublerMix, doublerMixKnob);
    doublerDetuneAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::doublerDetune, doublerDetuneKnob);
    doublerDelayAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::doublerDelay, doublerDelayKnob);
    doublerWidthAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::doublerWidth, doublerWidthKnob);
    delayMixAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::delayMix, delayMixKnob);
    delayTimeAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::delayTime, delayTimeKnob);
    delayFeedbackAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::delayFeedback, delayFeedbackKnob);
    delayFilterAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::delayFilterFreq, delayFilterKnob);
    delaySyncAttach = std::make_unique<ButtonAttachment>(p.apvts, ParamIDs::delaySync, delaySyncButton);
    delaySyncDivAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::delaySyncDiv, delaySyncDivKnob);
    reverbMixAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::reverbMix, reverbMixKnob);
    reverbSizeAttach = std::make_unique<SliderAttachment>(p.apvts, ParamIDs::reverbSize, reverbSizeKnob);

    initLabel(doublerLabel, "DOUBLER", this);
    initLabel(delayLabel, "DELAY", this);
    initLabel(reverbLabel, "REVERB", this);
}

void FXTabContent::setupKnob(juce::Slider& slider)
{
    initKnob(slider, this);
}

void FXTabContent::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    drawSectionBg(g, 5, 5, 320, 240, "DOUBLER");
    drawSectionBg(g, 330, 5, 360, 240, "DELAY");
    drawSectionBg(g, 5, 255, 200, 230, "REVERB");
}

void FXTabContent::resized()
{
    int knobSize = 60;
    int row1Y = 30;
    int row2Y = row1Y + knobSize + 20;

    // Doubler section
    doublerMixKnob.setBounds(15, row1Y, knobSize, knobSize);
    doublerDetuneKnob.setBounds(85, row1Y, knobSize, knobSize);
    doublerDelayKnob.setBounds(155, row1Y, knobSize, knobSize);
    doublerWidthKnob.setBounds(225, row1Y, knobSize, knobSize);

    // Delay section
    delayMixKnob.setBounds(340, row1Y, knobSize, knobSize);
    delayTimeKnob.setBounds(410, row1Y, knobSize, knobSize);
    delayFeedbackKnob.setBounds(480, row1Y, knobSize, knobSize);
    delayFilterKnob.setBounds(550, row1Y, knobSize, knobSize);
    delaySyncButton.setBounds(340, row2Y, 80, 22);
    delaySyncDivKnob.setBounds(430, row2Y, knobSize, knobSize);

    // Reverb section
    reverbMixKnob.setBounds(15, 280, knobSize, knobSize);
    reverbSizeKnob.setBounds(85, 280, knobSize, knobSize);
}

// ============================================================================
// MAIN EDITOR
// ============================================================================
ProVocalEditor::ProVocalEditor(ProVocalProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      presetBrowser(p.apvts),
      pitchTab(p),
      dynamicsTab(p),
      toneTab(p),
      fxTab(p),
      levelMeter(p)
{
    setLookAndFeel(&customLookAndFeel);
    setSize(1100, 700);

    // Preset browser in header
    addAndMakeVisible(presetBrowser);

    // Tab panel
    tabPanel.addTab("PITCH", &pitchTab);
    tabPanel.addTab("DYNAMICS", &dynamicsTab);
    tabPanel.addTab("TONE", &toneTab);
    tabPanel.addTab("FX", &fxTab);
    addAndMakeVisible(tabPanel);

    // Output section (persistent, right side)
    outputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 18);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, text);
    outputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(outputGainSlider);

    outputGainAttach = std::make_unique<SliderAttachment>(processorRef.apvts, ParamIDs::outputGain, outputGainSlider);

    outputLabel.setText("OUT", juce::dontSendNotification);
    outputLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    outputLabel.setColour(juce::Label::textColourId, knob);
    outputLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputLabel);

    addAndMakeVisible(levelMeter);
}

ProVocalEditor::~ProVocalEditor()
{
    setLookAndFeel(nullptr);
}

void ProVocalEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(bg);

    // Header bar
    g.setColour(sectionBg);
    g.fillRect(0, 0, getWidth(), 40);
    g.setColour(text);
    g.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    g.drawText("ProVocal", 15, 5, 120, 30, juce::Justification::centredLeft);
    g.setColour(dimText);
    g.setFont(11.0f);
    g.drawText("v2.0", 135, 12, 40, 16, juce::Justification::centredLeft);

    // Output section background
    int outX = getWidth() - 50;
    g.setColour(sectionBg.withAlpha(0.5f));
    g.fillRoundedRectangle(static_cast<float>(outX), 70.0f, 45.0f, 620.0f, 6.0f);
}

void ProVocalEditor::resized()
{
    int outSectionW = 50;
    int headerH = 40;

    // Preset browser in header
    presetBrowser.setBounds(250, 7, getWidth() - 310, 26);

    // Tab panel takes up main area
    tabPanel.setBounds(0, headerH, getWidth() - outSectionW, getHeight() - headerH);

    // Output section (right side, persistent)
    int outX = getWidth() - outSectionW;
    outputLabel.setBounds(outX, headerH + 5, outSectionW, 18);
    outputGainSlider.setBounds(outX + 5, headerH + 28, 30, getHeight() - headerH - 80);
    levelMeter.setBounds(outX + 37, headerH + 28, 10, getHeight() - headerH - 80);
}
