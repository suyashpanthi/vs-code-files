#include "ProVocalLookAndFeel.h"

using namespace ProVocalColours;

ProVocalLookAndFeel::ProVocalLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, knob);
    setColour(juce::Slider::rotarySliderOutlineColourId, accent);
    setColour(juce::Slider::thumbColourId, knob);
    setColour(juce::Label::textColourId, text);
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
    g.setColour(accent);
    g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));

    // Value arc
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f,
                            0.0f, rotaryStartAngle, angle, true);
    g.setColour(knob);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Pointer dot
    auto pointerLength = radius * 0.6f;
    auto pointerX = centreX + pointerLength * std::cos(angle - juce::MathConstants<float>::halfPi);
    auto pointerY = centreY + pointerLength * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.setColour(text);
    g.fillEllipse(pointerX - 3.0f, pointerY - 3.0f, 6.0f, 6.0f);

    // Center circle
    g.setColour(sectionBg);
    g.fillEllipse(centreX - radius * 0.4f, centreY - radius * 0.4f,
                  radius * 0.8f, radius * 0.8f);

    // Value text
    g.setColour(text);
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
        g.setColour(accent);
        g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, 2.0f);

        // Filled portion
        g.setColour(knob);
        g.fillRoundedRectangle(trackX, sliderPos, trackWidth, (float)(y + height) - sliderPos, 2.0f);

        // Thumb
        g.setColour(text);
        g.fillRoundedRectangle(trackX - 6.0f, sliderPos - 4.0f, trackWidth + 12.0f, 8.0f, 3.0f);
    }
    else
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos,
                                          minSliderPos, maxSliderPos, style, slider);
    }
}
