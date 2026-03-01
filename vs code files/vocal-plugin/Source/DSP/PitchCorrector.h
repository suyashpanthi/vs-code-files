#pragma once
#include <JuceHeader.h>
#include "../Utils/ScaleDefinitions.h"
#include <array>

class PitchCorrector
{
public:
    PitchCorrector();

    void prepare(double sampleRate);
    void reset();

    // Set target scale and root note
    void setScale(int rootNote, int scaleType);

    // Set note bypass (true = note is bypassed from correction)
    void setNoteBypass(int noteIndex, bool bypassed);

    // Set retune speed in milliseconds (0 = instant, 400 = slow)
    void setRetuneSpeed(float ms);

    // Set humanize amount (0-1)
    void setHumanize(float amount);

    // Set vibrato parameters
    void setVibrato(float rateHz, float depthCents);

    // Set MIDI mode target (negative = off)
    void setMidiTarget(float midiNoteNumber);

    // Set pitch bend (in semitones)
    void setPitchBend(float semitones);

    // Given detected pitch in Hz, compute the target pitch ratio
    // Returns the ratio by which pitch should be shifted
    float computeRatio(float detectedPitchHz);

    // Get the corrected pitch in Hz (for display)
    float getCorrectedPitch() const { return correctedPitchHz; }

private:
    double sr = 44100.0;

    int rootNote = 0;       // 0=C, 1=C#, ..., 11=B
    int scaleType = 0;      // Index into Scales::allScales
    float retuneSpeedMs = 50.0f;
    float humanizeAmount = 0.0f;
    float vibratoRate = 0.0f;
    float vibratoDepth = 0.0f;    // in cents
    float midiTarget = -1.0f;     // negative = off
    float pitchBendSemitones = 0.0f;

    std::array<bool, 12> noteBypass = {};  // true = bypassed

    // Smoothing
    float smoothedRatio = 1.0f;
    float smoothAlpha = 0.0f;

    // Vibrato LFO
    float vibratoPhase = 0.0f;

    // Random for humanize
    juce::Random rng;

    float correctedPitchHz = 0.0f;

    int sampleCounter = 0;

    float hzToMidiNote(float hz) const;
    float midiNoteToHz(float note) const;
};
