#include "PitchCorrector.h"
#include <cmath>

PitchCorrector::PitchCorrector() {}

void PitchCorrector::prepare(double sampleRate)
{
    sr = sampleRate;
    smoothedRatio = 1.0f;
    vibratoPhase = 0.0f;
    sampleCounter = 0;
    setRetuneSpeed(retuneSpeedMs);
}

void PitchCorrector::reset()
{
    smoothedRatio = 1.0f;
    vibratoPhase = 0.0f;
    sampleCounter = 0;
    correctedPitchHz = 0.0f;
}

void PitchCorrector::setScale(int root, int scale)
{
    rootNote = root % 12;
    scaleType = juce::jlimit(0, static_cast<int>(Scales::allScales.size()) - 1, scale);
}

void PitchCorrector::setNoteBypass(int noteIndex, bool bypassed)
{
    if (noteIndex >= 0 && noteIndex < 12)
        noteBypass[static_cast<size_t>(noteIndex)] = bypassed;
}

void PitchCorrector::setRetuneSpeed(float ms)
{
    retuneSpeedMs = ms;
    if (ms <= 0.0f)
        smoothAlpha = 1.0f; // instant
    else
        smoothAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sr) * ms / 1000.0f));
}

void PitchCorrector::setHumanize(float amount)
{
    humanizeAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void PitchCorrector::setVibrato(float rateHz, float depthCents)
{
    vibratoRate = rateHz;
    vibratoDepth = depthCents;
}

void PitchCorrector::setMidiTarget(float midiNoteNumber)
{
    midiTarget = midiNoteNumber;
}

void PitchCorrector::setPitchBend(float semitones)
{
    pitchBendSemitones = semitones;
}

float PitchCorrector::computeRatio(float detectedPitchHz)
{
    if (detectedPitchHz <= 0.0f)
    {
        correctedPitchHz = 0.0f;
        return 1.0f;
    }

    float detectedMidi = hzToMidiNote(detectedPitchHz);
    float targetMidi = detectedMidi;

    if (midiTarget >= 0.0f)
    {
        // MIDI mode: snap to the MIDI note + pitch bend
        targetMidi = midiTarget + pitchBendSemitones;
    }
    else
    {
        // Scale-based correction
        // Convert to semitone offset from root
        int noteInOctave = (static_cast<int>(std::round(detectedMidi)) % 12 + 12 - rootNote) % 12;

        // Check if this note is bypassed
        if (noteBypass[static_cast<size_t>(noteInOctave)])
        {
            // Don't correct bypassed notes
            targetMidi = detectedMidi;
        }
        else
        {
            const auto& scale = Scales::allScales[static_cast<size_t>(scaleType)];
            int correctedOffset = Scales::quantizeToScale(noteInOctave, scale);
            float correction = static_cast<float>(correctedOffset - noteInOctave);
            targetMidi = std::round(detectedMidi) + correction;
        }

        // Apply pitch bend
        targetMidi += pitchBendSemitones;
    }

    // Humanize: add small random offset
    if (humanizeAmount > 0.0f)
    {
        float maxDeviation = humanizeAmount * 0.5f; // max ±50 cents at 100%
        float randomOffset = (rng.nextFloat() * 2.0f - 1.0f) * maxDeviation;
        targetMidi += randomOffset;
    }

    // Vibrato LFO
    if (vibratoRate > 0.0f && vibratoDepth > 0.0f)
    {
        float vibratoSemitones = vibratoDepth / 100.0f; // cents to semitones
        float lfo = std::sin(2.0f * juce::MathConstants<float>::pi * vibratoPhase);
        targetMidi += lfo * vibratoSemitones;

        // Advance vibrato phase (called at hop rate, not sample rate)
        vibratoPhase += vibratoRate / static_cast<float>(sr) * 512.0f;
        if (vibratoPhase > 1.0f)
            vibratoPhase -= 1.0f;
    }

    float targetHz = midiNoteToHz(targetMidi);
    float targetRatio = targetHz / detectedPitchHz;

    // Smooth the ratio with one-pole lowpass
    smoothedRatio = smoothedRatio + smoothAlpha * (targetRatio - smoothedRatio);

    correctedPitchHz = detectedPitchHz * smoothedRatio;
    return smoothedRatio;
}

float PitchCorrector::hzToMidiNote(float hz) const
{
    return 69.0f + 12.0f * std::log2(hz / 440.0f);
}

float PitchCorrector::midiNoteToHz(float note) const
{
    return 440.0f * std::pow(2.0f, (note - 69.0f) / 12.0f);
}
