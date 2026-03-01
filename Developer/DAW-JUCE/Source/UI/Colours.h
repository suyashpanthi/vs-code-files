#pragma once
#include <JuceHeader.h>

namespace DAWColours
{
    // Main backgrounds
    const juce::Colour background       { 0xFF1E1E1E };
    const juce::Colour darkBackground   { 0xFF141414 };
    const juce::Colour panelBackground  { 0xFF2A2A2A };
    const juce::Colour headerBackground { 0xFF333333 };

    // Borders / separators
    const juce::Colour border           { 0xFF3C3C3C };
    const juce::Colour borderLight      { 0xFF505050 };

    // Text
    const juce::Colour textPrimary      { 0xFFE0E0E0 };
    const juce::Colour textSecondary    { 0xFF999999 };
    const juce::Colour textDim          { 0xFF666666 };

    // Accent
    const juce::Colour accent           { 0xFF4A90D9 };
    const juce::Colour accentBright     { 0xFF5BA3F5 };

    // Transport
    const juce::Colour playGreen        { 0xFF4CAF50 };
    const juce::Colour stopRed          { 0xFFE53935 };
    const juce::Colour recordRed        { 0xFFFF1744 };
    const juce::Colour pauseYellow      { 0xFFFFC107 };

    // Track colours (cycle through for new tracks)
    const juce::Colour trackColours[] = {
        juce::Colour (0xFF4A90D9u),  // blue
        juce::Colour (0xFF66BB6Au),  // green
        juce::Colour (0xFFEF5350u),  // red
        juce::Colour (0xFFAB47BCu),  // purple
        juce::Colour (0xFFFFA726u),  // orange
        juce::Colour (0xFF26C6DAu),  // cyan
        juce::Colour (0xFFEC407Au),  // pink
        juce::Colour (0xFF8D6E63u),  // brown
    };
    const int numTrackColours = 8;

    // Meters
    const juce::Colour meterGreen       { 0xFF4CAF50 };
    const juce::Colour meterYellow      { 0xFFFFC107 };
    const juce::Colour meterRed         { 0xFFFF5252 };
    const juce::Colour meterBackground  { 0xFF1A1A1A };

    // Buttons
    const juce::Colour muteColour       { 0xFFFFC107 };
    const juce::Colour soloColour       { 0xFF4CAF50 };
    const juce::Colour recordColour     { 0xFFFF1744 };

    // Waveform
    const juce::Colour waveformFill     { 0xFF4A90D9 };
    const juce::Colour waveformOutline  { 0xFF6BB0FF };
    const juce::Colour regionBackground { 0xFF2C3E50 };

    // Playhead
    const juce::Colour playhead         { 0xFFFFFFFF };
}
