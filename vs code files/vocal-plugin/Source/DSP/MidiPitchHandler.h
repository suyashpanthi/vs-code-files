#pragma once
#include <JuceHeader.h>
#include <array>

// Handles MIDI note messages and converts to target pitch for the corrector
class MidiPitchHandler
{
public:
    MidiPitchHandler();

    void reset();

    // Process a MIDI buffer and extract note/pitch-bend info
    void processMidi(const juce::MidiBuffer& midiMessages);

    // Set pitch bend range in semitones (default 2)
    void setPitchBendRange(int semitones);

    // Returns true if any MIDI note is currently held
    bool hasActiveNote() const { return activeNoteCount > 0; }

    // Get the current target MIDI note number (float to include pitch bend)
    float getTargetMidiNote() const;

    // Get pitch bend in semitones
    float getPitchBendSemitones() const { return pitchBendSemitones; }

private:
    std::array<bool, 128> noteStates {};
    int activeNoteCount = 0;
    int lastNoteOn = -1;           // Most recent note-on for monophonic behavior
    int pitchBendRange = 2;        // Semitones
    float pitchBendSemitones = 0.0f;
};
