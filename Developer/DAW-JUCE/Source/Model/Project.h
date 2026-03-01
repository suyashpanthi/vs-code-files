#pragma once
#include <JuceHeader.h>
#include "Track.h"

class Project
{
public:
    Project();

    const juce::String& getName() const { return name; }
    void setName (const juce::String& n) { name = n; }

    double getSampleRate() const { return sampleRate; }
    void setSampleRate (double sr) { sampleRate = sr; }

    juce::OwnedArray<Track>& getTracks() { return tracks; }
    const juce::OwnedArray<Track>& getTracks() const { return tracks; }

    Track* addTrack (const juce::String& name);
    void removeTrack (int index);

    const juce::File& getProjectFile() const { return projectFile; }
    void setProjectFile (const juce::File& f) { projectFile = f; }

    std::unique_ptr<juce::XmlElement> toXml() const;
    static std::unique_ptr<Project> fromXml (const juce::XmlElement& xml,
                                              juce::AudioFormatManager& formatManager);

private:
    juce::String name { "Untitled" };
    double sampleRate = 44100.0;
    juce::OwnedArray<Track> tracks;
    juce::File projectFile;
    int nextTrackColourIndex = 0;
};
