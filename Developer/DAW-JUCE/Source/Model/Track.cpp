#include "Track.h"

Track::Track (const juce::String& n, juce::Colour c) : name (n), colour (c) {}

AudioRegion* Track::addRegion (const juce::File& file, int64_t positionInSamples,
                               juce::AudioFormatManager& formatManager)
{
    auto region = std::make_unique<AudioRegion> (file, formatManager);
    if (region->getFormatReader() == nullptr)
        return nullptr;

    region->setPositionInSamples (positionInSamples);
    region->setColour (colour);
    auto* ptr = region.get();
    regions.add (region.release());
    return ptr;
}

std::unique_ptr<juce::XmlElement> Track::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement> ("TRACK");
    xml->setAttribute ("name", name);
    xml->setAttribute ("colour", (int) colour.getARGB());
    xml->setAttribute ("volume", (double) volume);
    xml->setAttribute ("pan", (double) pan);
    xml->setAttribute ("muted", muted);
    xml->setAttribute ("solo", solo);
    xml->setAttribute ("armed", armed);

    for (auto* region : regions)
        xml->addChildElement (region->toXml().release());

    return xml;
}

std::unique_ptr<Track> Track::fromXml (const juce::XmlElement& xml,
                                        juce::AudioFormatManager& formatManager)
{
    auto track = std::make_unique<Track> (
        xml.getStringAttribute ("name", "Track"),
        juce::Colour ((uint32_t) xml.getIntAttribute ("colour", (int) 0xFF4A90D9))
    );

    track->setVolume ((float) xml.getDoubleAttribute ("volume", 1.0));
    track->setPan ((float) xml.getDoubleAttribute ("pan", 0.0));
    track->setMuted (xml.getBoolAttribute ("muted", false));
    track->setSolo (xml.getBoolAttribute ("solo", false));
    track->setArmed (xml.getBoolAttribute ("armed", false));

    for (auto* regionXml : xml.getChildWithTagNameIterator ("REGION"))
    {
        auto region = AudioRegion::fromXml (*regionXml, formatManager);
        if (region != nullptr)
            track->getRegions().add (region.release());
    }

    return track;
}
