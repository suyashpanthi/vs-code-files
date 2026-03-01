#pragma once
#include <JuceHeader.h>

class AudioRegion
{
public:
    AudioRegion (const juce::File& audioFile, juce::AudioFormatManager& formatManager);
    ~AudioRegion();

    const juce::File& getFile() const { return file; }
    juce::String getName() const { return name; }
    void setName (const juce::String& n) { name = n; }

    // Timeline position (where this region sits on the timeline)
    int64_t getPositionInSamples() const { return positionInSamples; }
    void setPositionInSamples (int64_t pos) { positionInSamples = pos; }

    // Source offset (where to start reading from the source file)
    int64_t getSourceOffsetInSamples() const { return sourceOffset; }
    void setSourceOffsetInSamples (int64_t offset) { sourceOffset = offset; }

    // Length (how much of the source to use)
    int64_t getLengthInSamples() const { return lengthInSamples; }
    void setLengthInSamples (int64_t len) { lengthInSamples = len; }

    // Gain
    float getGain() const { return gain; }
    void setGain (float g) { gain = g; }

    // Colour for display
    juce::Colour getColour() const { return colour; }
    void setColour (juce::Colour c) { colour = c; }

    // Access to the reader (shared, do not delete)
    juce::AudioFormatReader* getFormatReader() const { return reader.get(); }

    // Total length of source file
    int64_t getTotalSourceLength() const;

    double getSampleRate() const;

    // Serialization
    std::unique_ptr<juce::XmlElement> toXml() const;
    static std::unique_ptr<AudioRegion> fromXml (const juce::XmlElement& xml,
                                                  juce::AudioFormatManager& formatManager);

private:
    juce::File file;
    juce::String name;
    std::unique_ptr<juce::AudioFormatReader> reader;

    int64_t positionInSamples = 0;
    int64_t sourceOffset = 0;
    int64_t lengthInSamples = 0;
    float gain = 1.0f;
    juce::Colour colour { 0xFF4A90D9 };
};
