#pragma once
#include <JuceHeader.h>

// Shared color constants
namespace ProVocalColours
{
    inline const juce::Colour bg         (0xff1a1a2e);
    inline const juce::Colour sectionBg  (0xff16213e);
    inline const juce::Colour accent     (0xff0f3460);
    inline const juce::Colour knob       (0xffe94560);
    inline const juce::Colour text       (0xffeaeaea);
    inline const juce::Colour dimText    (0xff8a8a9a);
    inline const juce::Colour meterGreen (0xff00e676);
    inline const juce::Colour meterYellow(0xffffc107);
    inline const juce::Colour meterRed   (0xffff1744);
    inline const juce::Colour tabActive  (0xff2a2a4e);
    inline const juce::Colour pitchLine  (0xff4fc3f7);
    inline const juce::Colour pitchCorrected(0xffff7043);
}

class ProVocalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ProVocalLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
};
