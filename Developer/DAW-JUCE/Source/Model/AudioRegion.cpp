#include "AudioRegion.h"

AudioRegion::AudioRegion (const juce::File& audioFile, juce::AudioFormatManager& formatManager)
    : file (audioFile),
      name (audioFile.getFileNameWithoutExtension())
{
    reader.reset (formatManager.createReaderFor (audioFile));
    if (reader != nullptr)
        lengthInSamples = (int64_t) reader->lengthInSamples;
}

AudioRegion::~AudioRegion() {}

int64_t AudioRegion::getTotalSourceLength() const
{
    return reader != nullptr ? (int64_t) reader->lengthInSamples : 0;
}

double AudioRegion::getSampleRate() const
{
    return reader != nullptr ? reader->sampleRate : 44100.0;
}

std::unique_ptr<juce::XmlElement> AudioRegion::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement> ("REGION");
    xml->setAttribute ("file", file.getFullPathName());
    xml->setAttribute ("name", name);
    xml->setAttribute ("position", (int) positionInSamples);
    xml->setAttribute ("offset", (int) sourceOffset);
    xml->setAttribute ("length", (int) lengthInSamples);
    xml->setAttribute ("gain", (double) gain);
    xml->setAttribute ("colour", (int) colour.getARGB());
    return xml;
}

std::unique_ptr<AudioRegion> AudioRegion::fromXml (const juce::XmlElement& xml,
                                                    juce::AudioFormatManager& formatManager)
{
    auto file = juce::File (xml.getStringAttribute ("file"));
    if (! file.existsAsFile())
        return nullptr;

    auto region = std::make_unique<AudioRegion> (file, formatManager);
    region->setName (xml.getStringAttribute ("name", file.getFileNameWithoutExtension()));
    region->setPositionInSamples (xml.getIntAttribute ("position", 0));
    region->setSourceOffsetInSamples (xml.getIntAttribute ("offset", 0));
    region->setLengthInSamples (xml.getIntAttribute ("length", (int) region->getTotalSourceLength()));
    region->setGain ((float) xml.getDoubleAttribute ("gain", 1.0));
    region->setColour (juce::Colour ((uint32_t) xml.getIntAttribute ("colour", (int) 0xFF4A90D9)));
    return region;
}
