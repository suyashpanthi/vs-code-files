#include "PluginEditor.h"

// ============================================================================
// Custom LookAndFeel
// ============================================================================
static const juce::Colour bgColour(0xff1a1a2e);
static const juce::Colour sectionBg(0xff16213e);
static const juce::Colour accentColour(0xff0f3460);
static const juce::Colour knobColour(0xffe94560);
static const juce::Colour textColour(0xffeaeaea);
static const juce::Colour dimTextColour(0xff8a8a9a);
static const juce::Colour meterGreen(0xff00e676);
static const juce::Colour meterYellow(0xffffc107);
static const juce::Colour meterRed(0xffff1744);

ProVocalLookAndFeel::ProVocalLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, knobColour);
    setColour(juce::Slider::rotarySliderOutlineColourId, accentColour);
    setColour(juce::Slider::thumbColourId, knobColour);
    setColour(juce::Label::textColourId, textColour);
}

void ProVocalLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background arc
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f,
                         0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(accentColour);
    g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // Value arc
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f,
                            0.0f, rotaryStartAngle, angle, true);
    g.setColour(knobColour);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Pointer dot
    auto pointerLength = radius * 0.6f;
    auto pointerX = centreX + pointerLength * std::cos(angle - juce::MathConstants<float>::halfPi);
    auto pointerY = centreY + pointerLength * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.setColour(textColour);
    g.fillEllipse(pointerX - 3.0f, pointerY - 3.0f, 6.0f, 6.0f);

    // Center circle
    g.setColour(sectionBg);
    g.fillEllipse(centreX - radius * 0.4f, centreY - radius * 0.4f,
                  radius * 0.8f, radius * 0.8f);

    // Value text
    g.setColour(textColour);
    g.setFont(10.0f);
    auto valText = juce::String(slider.getValue(), 1);
    g.drawText(valText, bounds.toNearestInt(), juce::Justification::centred, false);
}

void ProVocalLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                            juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        auto trackWidth = 4.0f;
        auto trackX = (float)x + (float)width * 0.5f - trackWidth * 0.5f;

        // Track background
        g.setColour(accentColour);
        g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, 2.0f);

        // Filled portion
        g.setColour(knobColour);
        g.fillRoundedRectangle(trackX, sliderPos, trackWidth, (float)(y + height) - sliderPos, 2.0f);

        // Thumb
        g.setColour(textColour);
        g.fillRoundedRectangle(trackX - 6.0f, sliderPos - 4.0f, trackWidth + 12.0f, 8.0f, 3.0f);
    }
    else
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos,
                                          minSliderPos, maxSliderPos, style, slider);
    }
}

// ============================================================================
// Level Meter
// ============================================================================
LevelMeter::LevelMeter(ProVocalProcessor& p) : processor(p)
{
    startTimerHz(30);
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    g.setColour(juce::Colour(0xff0d0d1a));
    g.fillRoundedRectangle(bounds, 3.0f);

    float meterHeight = bounds.getHeight() * juce::jmin(level, 1.0f);
    auto meterBounds = bounds.removeFromBottom(meterHeight);

    juce::Colour meterCol = meterGreen;
    if (level > 0.9f) meterCol = meterRed;
    else if (level > 0.7f) meterCol = meterYellow;

    g.setColour(meterCol);
    g.fillRoundedRectangle(meterBounds, 3.0f);
}

void LevelMeter::timerCallback()
{
    float newLevel = processor.getCurrentOutputLevel();
    level = level * 0.85f + newLevel * 0.15f; // smooth
    repaint();
}

// ============================================================================
// Editor
// ============================================================================
ProVocalEditor::ProVocalEditor(ProVocalProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), levelMeter(p)
{
    setLookAndFeel(&customLookAndFeel);
    setSize(700, 500);

    // Setup all knobs
    setupKnob(hpfFreqKnob);

    setupKnob(compThreshKnob);
    setupKnob(compRatioKnob);
    setupKnob(compAttackKnob);
    setupKnob(compReleaseKnob);
    setupKnob(compMakeupKnob);

    setupKnob(eqLowGainKnob);
    setupKnob(eqMidGainKnob);
    setupKnob(eqMidFreqKnob);
    setupKnob(eqHighGainKnob);

    setupKnob(deEssThreshKnob);
    setupKnob(deEssFreqKnob);

    setupKnob(satDriveKnob);

    setupKnob(reverbMixKnob);
    setupKnob(reverbSizeKnob);

    // Output gain as vertical slider
    outputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, textColour);
    outputGainSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(outputGainSlider);

    // Attach all parameters
    hpfFreqAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "hpfFreq", hpfFreqKnob);

    compThreshAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "compThresh", compThreshKnob);
    compRatioAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "compRatio", compRatioKnob);
    compAttackAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "compAttack", compAttackKnob);
    compReleaseAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "compRelease", compReleaseKnob);
    compMakeupAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "compMakeup", compMakeupKnob);

    eqLowGainAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "eqLowGain", eqLowGainKnob);
    eqMidGainAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "eqMidGain", eqMidGainKnob);
    eqMidFreqAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "eqMidFreq", eqMidFreqKnob);
    eqHighGainAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "eqHighGain", eqHighGainKnob);

    deEssThreshAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "deEssThresh", deEssThreshKnob);
    deEssFreqAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "deEssFreq", deEssFreqKnob);

    satDriveAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "satDrive", satDriveKnob);

    reverbMixAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "reverbMix", reverbMixKnob);
    reverbSizeAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "reverbSize", reverbSizeKnob);

    outputGainAttach = std::make_unique<SliderAttachment>(processorRef.apvts, "outputGain", outputGainSlider);

    // Section labels
    setupLabel(hpfLabel, "HPF");
    setupLabel(compLabel, "COMPRESSOR");
    setupLabel(eqLabel, "EQ");
    setupLabel(deEssLabel, "DE-ESSER");
    setupLabel(satLabel, "SATURATE");
    setupLabel(reverbLabel, "REVERB");
    setupLabel(outputLabel, "OUTPUT");

    addAndMakeVisible(levelMeter);
}

ProVocalEditor::~ProVocalEditor()
{
    setLookAndFeel(nullptr);
}

void ProVocalEditor::setupKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 16);
    slider.setColour(juce::Slider::textBoxTextColourId, textColour);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);
}

void ProVocalEditor::setupLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, knobColour);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void ProVocalEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(bgColour);

    // Title bar
    g.setColour(sectionBg);
    g.fillRect(0, 0, getWidth(), 40);
    g.setColour(textColour);
    g.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    g.drawText("ProVocal", 15, 5, 200, 30, juce::Justification::centredLeft);
    g.setColour(dimTextColour);
    g.setFont(12.0f);
    g.drawText("Professional Vocal Chain", 150, 10, 200, 20, juce::Justification::centredLeft);

    // Section backgrounds
    auto drawSection = [&](int x, int w)
    {
        g.setColour(sectionBg.withAlpha(0.5f));
        g.fillRoundedRectangle((float)x, 50.0f, (float)w, 440.0f, 6.0f);
    };

    // Section widths: HPF(65), COMP(175), EQ(155), DEESS(90), SAT(65), REVERB(90), OUTPUT(55)
    drawSection(5, 65);
    drawSection(75, 175);
    drawSection(255, 155);
    drawSection(415, 90);
    drawSection(510, 65);
    drawSection(580, 90);
    drawSection(675, 25);
}

void ProVocalEditor::resized()
{
    int knobSize = 55;
    int labelH = 20;
    int topY = 55;
    int row1Y = topY + labelH + 5;
    int row2Y = row1Y + knobSize + 10;


    // HPF section (x=5, w=65)
    hpfLabel.setBounds(5, topY, 65, labelH);
    hpfFreqKnob.setBounds(10, row1Y, knobSize, knobSize);

    // Compressor section (x=75, w=175)
    compLabel.setBounds(75, topY, 175, labelH);
    compThreshKnob.setBounds(80, row1Y, knobSize, knobSize);
    compRatioKnob.setBounds(140, row1Y, knobSize, knobSize);
    compAttackKnob.setBounds(200, row1Y, knobSize, knobSize);
    compReleaseKnob.setBounds(80, row2Y, knobSize, knobSize);
    compMakeupKnob.setBounds(140, row2Y, knobSize, knobSize);

    // EQ section (x=255, w=155)
    eqLabel.setBounds(255, topY, 155, labelH);
    eqLowGainKnob.setBounds(260, row1Y, knobSize, knobSize);
    eqMidGainKnob.setBounds(320, row1Y, knobSize, knobSize);
    eqMidFreqKnob.setBounds(260, row2Y, knobSize, knobSize);
    eqHighGainKnob.setBounds(320, row2Y, knobSize, knobSize);

    // De-Esser section (x=415, w=90)
    deEssLabel.setBounds(415, topY, 90, labelH);
    deEssThreshKnob.setBounds(420, row1Y, knobSize, knobSize);
    deEssFreqKnob.setBounds(420, row2Y, knobSize, knobSize);

    // Saturation section (x=510, w=65)
    satLabel.setBounds(510, topY, 65, labelH);
    satDriveKnob.setBounds(515, row1Y, knobSize, knobSize);

    // Reverb section (x=580, w=90)
    reverbLabel.setBounds(580, topY, 90, labelH);
    reverbMixKnob.setBounds(585, row1Y, knobSize, knobSize);
    reverbSizeKnob.setBounds(585, row2Y, knobSize, knobSize);

    // Output section (far right)
    outputLabel.setBounds(650, topY, 45, labelH);
    outputGainSlider.setBounds(655, row1Y, 30, knobSize * 3);
    levelMeter.setBounds(688, row1Y, 8, knobSize * 3);
}
