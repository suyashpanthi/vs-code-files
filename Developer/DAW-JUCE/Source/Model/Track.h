#pragma once
#include <JuceHeader.h>
#include "AudioRegion.h"

class Track
{
public:
    Track (const juce::String& name, juce::Colour colour);

    const juce::String& getName() const { return name; }
    void setName (const juce::String& n) { name = n; }

    juce::Colour getColour() const { return colour; }
    void setColour (juce::Colour c) { colour = c; }

    float getVolume() const { return volume; }
    void setVolume (float v) { volume = v; }

    float getPan() const { return pan; }
    void setPan (float p) { pan = p; }

    bool isMuted() const { return muted; }
    void setMuted (bool m) { muted = m; }

    bool isSolo() const { return solo; }
    void setSolo (bool s) { solo = s; }

    bool isArmed() const { return armed; }
    void setArmed (bool a) { armed = a; }

    juce::OwnedArray<AudioRegion>& getRegions() { return regions; }
    const juce::OwnedArray<AudioRegion>& getRegions() const { return regions; }

    AudioRegion* addRegion (const juce::File& file, int64_t positionInSamples,
                            juce::AudioFormatManager& formatManager);

    std::unique_ptr<juce::XmlElement> toXml() const;
    static std::unique_ptr<Track> fromXml (const juce::XmlElement& xml,
                                           juce::AudioFormatManager& formatManager);

private:
    juce::String name;
    juce::Colour colour;
    float volume = 1.0f;
    float pan = 0.0f;
    bool muted = false;
    bool solo = false;
    bool armed = false;
    juce::OwnedArray<AudioRegion> regions;
};
