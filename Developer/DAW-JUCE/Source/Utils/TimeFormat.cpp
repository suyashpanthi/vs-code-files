#include "TimeFormat.h"

namespace TimeFormat
{

juce::String samplesToTimecode (int64_t samples, double sampleRate)
{
    if (sampleRate <= 0.0)
        return "00:00:00.000";

    double totalSeconds = (double) samples / sampleRate;
    int hours   = (int) (totalSeconds / 3600.0);
    int minutes = (int) (std::fmod (totalSeconds, 3600.0) / 60.0);
    int seconds = (int) std::fmod (totalSeconds, 60.0);
    int millis  = (int) (std::fmod (totalSeconds, 1.0) * 1000.0);

    return juce::String::formatted ("%02d:%02d:%02d.%03d", hours, minutes, seconds, millis);
}

juce::String samplesToBarsBeatsTicks (int64_t samples, double sampleRate, double bpm, int timeSigNum, int /*timeSigDen*/)
{
    if (sampleRate <= 0.0 || bpm <= 0.0)
        return "1.1.000";

    double totalBeats = ((double) samples / sampleRate) * (bpm / 60.0);
    int totalBeatInt = (int) totalBeats;

    int bar  = totalBeatInt / timeSigNum + 1;
    int beat = totalBeatInt % timeSigNum + 1;
    int ticks = (int) ((totalBeats - (double) totalBeatInt) * 960.0);

    return juce::String::formatted ("%d.%d.%03d", bar, beat, ticks);
}

}
