#pragma once
#include <array>

namespace Scales
{
    // Each scale is a 12-element bool array: true = note is in scale
    // Index 0 = root, 1 = root+1 semitone, etc.
    using ScalePattern = std::array<bool, 12>;

    // Scale indices (matching pitchScale parameter choices)
    enum ScaleType
    {
        Chromatic = 0,
        Major,
        NaturalMinor,
        HarmonicMinor,
        MelodicMinor,
        Pentatonic,
        MinorPentatonic,
        Blues,
        Dorian
    };

    inline constexpr ScalePattern chromatic       = {true, true, true, true, true, true, true, true, true, true, true, true};
    inline constexpr ScalePattern major           = {true, false, true, false, true, true, false, true, false, true, false, true};
    inline constexpr ScalePattern naturalMinor    = {true, false, true, true, false, true, false, true, true, false, true, false};
    inline constexpr ScalePattern harmonicMinor   = {true, false, true, true, false, true, false, true, true, false, false, true};
    inline constexpr ScalePattern melodicMinor    = {true, false, true, true, false, true, false, true, false, true, false, true};
    inline constexpr ScalePattern pentatonic      = {true, false, true, false, true, false, false, true, false, true, false, false};
    inline constexpr ScalePattern minorPentatonic = {true, false, false, true, false, true, false, true, false, false, true, false};
    inline constexpr ScalePattern blues           = {true, false, false, true, false, true, true, true, false, false, true, false};
    inline constexpr ScalePattern dorian          = {true, false, true, true, false, true, false, true, false, true, true, false};

    inline constexpr std::array<ScalePattern, 9> allScales = {
        chromatic, major, naturalMinor, harmonicMinor, melodicMinor,
        pentatonic, minorPentatonic, blues, dorian
    };

    inline const char* scaleNames[] = {
        "Chromatic", "Major", "Natural Minor", "Harmonic Minor", "Melodic Minor",
        "Pentatonic", "Minor Pentatonic", "Blues", "Dorian"
    };

    inline const char* noteNames[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    // Returns the nearest scale note (in semitones from root) for a given
    // chromatic offset from root (0-11). Returns the input if it's in the scale.
    inline int quantizeToScale(int semitoneFromRoot, const ScalePattern& scale)
    {
        int note = ((semitoneFromRoot % 12) + 12) % 12;
        if (scale[static_cast<size_t>(note)])
            return semitoneFromRoot;

        // Search outward for nearest scale note
        for (int offset = 1; offset <= 6; ++offset)
        {
            int above = (note + offset) % 12;
            int below = ((note - offset) % 12 + 12) % 12;
            if (scale[static_cast<size_t>(above)])
                return semitoneFromRoot + offset;
            if (scale[static_cast<size_t>(below)])
                return semitoneFromRoot - offset;
        }
        return semitoneFromRoot; // fallback (chromatic)
    }
}
