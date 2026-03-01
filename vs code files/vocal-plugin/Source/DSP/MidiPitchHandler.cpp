#include "MidiPitchHandler.h"

MidiPitchHandler::MidiPitchHandler() {}

void MidiPitchHandler::reset()
{
    noteStates.fill(false);
    activeNoteCount = 0;
    lastNoteOn = -1;
    pitchBendSemitones = 0.0f;
}

void MidiPitchHandler::processMidi(const juce::MidiBuffer& midiMessages)
{
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            if (!noteStates[static_cast<size_t>(note)])
            {
                noteStates[static_cast<size_t>(note)] = true;
                activeNoteCount++;
            }
            lastNoteOn = note;
        }
        else if (msg.isNoteOff())
        {
            int note = msg.getNoteNumber();
            if (noteStates[static_cast<size_t>(note)])
            {
                noteStates[static_cast<size_t>(note)] = false;
                activeNoteCount--;

                // If the released note was the last note-on, find another held note
                if (note == lastNoteOn)
                {
                    lastNoteOn = -1;
                    // Find highest held note (typical monophonic behavior)
                    for (int i = 127; i >= 0; --i)
                    {
                        if (noteStates[static_cast<size_t>(i)])
                        {
                            lastNoteOn = i;
                            break;
                        }
                    }
                }
            }
        }
        else if (msg.isPitchWheel())
        {
            // Convert 14-bit pitch wheel (0-16383, center=8192) to semitones
            int pitchWheelValue = msg.getPitchWheelValue();
            float normalized = (static_cast<float>(pitchWheelValue) - 8192.0f) / 8192.0f;
            pitchBendSemitones = normalized * static_cast<float>(pitchBendRange);
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            noteStates.fill(false);
            activeNoteCount = 0;
            lastNoteOn = -1;
        }
    }
}

void MidiPitchHandler::setPitchBendRange(int semitones)
{
    pitchBendRange = juce::jlimit(1, 24, semitones);
}

float MidiPitchHandler::getTargetMidiNote() const
{
    if (lastNoteOn >= 0)
        return static_cast<float>(lastNoteOn) + pitchBendSemitones;
    return -1.0f;
}
