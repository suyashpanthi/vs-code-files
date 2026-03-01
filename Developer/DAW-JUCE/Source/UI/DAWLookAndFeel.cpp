#include "DAWLookAndFeel.h"
#include "Colours.h"

DAWLookAndFeel::DAWLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, DAWColours::background);
    setColour (juce::TextButton::buttonColourId,          DAWColours::panelBackground);
    setColour (juce::TextButton::textColourOffId,         DAWColours::textPrimary);
    setColour (juce::TextButton::textColourOnId,          DAWColours::textPrimary);
    setColour (juce::ComboBox::backgroundColourId,        DAWColours::panelBackground);
    setColour (juce::ComboBox::textColourId,              DAWColours::textPrimary);
    setColour (juce::ComboBox::outlineColourId,           DAWColours::border);
    setColour (juce::PopupMenu::backgroundColourId,       DAWColours::panelBackground);
    setColour (juce::PopupMenu::textColourId,             DAWColours::textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, DAWColours::accent);
    setColour (juce::Label::textColourId,                 DAWColours::textPrimary);
    setColour (juce::TextEditor::backgroundColourId,      DAWColours::darkBackground);
    setColour (juce::TextEditor::textColourId,            DAWColours::textPrimary);
    setColour (juce::TextEditor::outlineColourId,         DAWColours::border);
    setColour (juce::ScrollBar::thumbColourId,            DAWColours::borderLight);
    setColour (juce::Slider::thumbColourId,               DAWColours::accent);
    setColour (juce::Slider::trackColourId,               DAWColours::border);
    setColour (juce::Slider::backgroundColourId,          DAWColours::darkBackground);
}

void DAWLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto baseColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColour = baseColour.brighter (0.1f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter (0.05f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (DAWColours::border);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void DAWLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                       juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto trackWidth = style == juce::Slider::LinearVertical ? 4.0f : 4.0f;

    if (style == juce::Slider::LinearVertical)
    {
        auto trackX = (float) x + (float) width * 0.5f;
        g.setColour (DAWColours::darkBackground);
        g.fillRoundedRectangle (trackX - trackWidth * 0.5f, (float) y, trackWidth, (float) height, 2.0f);

        g.setColour (slider.findColour (juce::Slider::thumbColourId));
        g.fillRoundedRectangle (trackX - trackWidth * 0.5f, sliderPos, trackWidth, (float) (y + height) - sliderPos, 2.0f);

        // Thumb
        g.setColour (DAWColours::textPrimary);
        g.fillRoundedRectangle (trackX - 8.0f, sliderPos - 4.0f, 16.0f, 8.0f, 3.0f);
    }
    else
    {
        auto trackY = (float) y + (float) height * 0.5f;
        g.setColour (DAWColours::darkBackground);
        g.fillRoundedRectangle ((float) x, trackY - trackWidth * 0.5f, (float) width, trackWidth, 2.0f);

        g.setColour (slider.findColour (juce::Slider::thumbColourId));
        g.fillRoundedRectangle ((float) x, trackY - trackWidth * 0.5f, sliderPos - (float) x, trackWidth, 2.0f);

        g.setColour (DAWColours::textPrimary);
        g.fillEllipse (sliderPos - 6.0f, trackY - 6.0f, 12.0f, 12.0f);
    }
}

void DAWLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider&)
{
    auto radius = (float) juce::jmin (width, height) * 0.5f - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Background arc
    juce::Path bgArc;
    bgArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (DAWColours::darkBackground);
    g.strokePath (bgArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Value arc
    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (DAWColours::accent);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Pointer
    juce::Path pointer;
    auto pointerLength = radius * 0.5f;
    auto pointerThickness = 2.5f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength, 1.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (DAWColours::textPrimary);
    g.fillPath (pointer);
}

void DAWLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));

    if (! label.isBeingEdited())
    {
        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());
        g.setColour (label.findColour (juce::Label::textColourId));
        g.setFont (label.getFont());
        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          juce::jmax (1, (int) ((float) textArea.getHeight() / label.getFont().getHeight())),
                          label.getMinimumHorizontalScale());
    }
}
