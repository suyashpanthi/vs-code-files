#include "Project.h"
#include "../UI/Colours.h"

Project::Project() {}

Track* Project::addTrack (const juce::String& trackName)
{
    auto colour = DAWColours::trackColours[nextTrackColourIndex % DAWColours::numTrackColours];
    nextTrackColourIndex++;

    auto track = std::make_unique<Track> (trackName, colour);
    auto* ptr = track.get();
    tracks.add (track.release());
    return ptr;
}

void Project::removeTrack (int index)
{
    if (index >= 0 && index < tracks.size())
        tracks.remove (index);
}

std::unique_ptr<juce::XmlElement> Project::toXml() const
{
    auto xml = std::make_unique<juce::XmlElement> ("DAW_PROJECT");
    xml->setAttribute ("name", name);
    xml->setAttribute ("sampleRate", sampleRate);

    for (auto* track : tracks)
        xml->addChildElement (track->toXml().release());

    return xml;
}

std::unique_ptr<Project> Project::fromXml (const juce::XmlElement& xml,
                                            juce::AudioFormatManager& formatManager)
{
    auto project = std::make_unique<Project>();
    project->setName (xml.getStringAttribute ("name", "Untitled"));
    project->setSampleRate (xml.getDoubleAttribute ("sampleRate", 44100.0));

    for (auto* trackXml : xml.getChildWithTagNameIterator ("TRACK"))
    {
        auto track = Track::fromXml (*trackXml, formatManager);
        if (track != nullptr)
            project->getTracks().add (track.release());
    }

    return project;
}
