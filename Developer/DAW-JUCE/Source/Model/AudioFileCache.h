#pragma once
#include <JuceHeader.h>
#include <map>

class AudioFileCache
{
public:
    AudioFileCache (juce::AudioFormatManager& fm) : formatManager (fm) {}

    juce::AudioFormatReader* getReaderFor (const juce::File& file)
    {
        auto key = file.getFullPathName().toStdString();
        auto it = cache.find (key);
        if (it != cache.end())
            return it->second.get();

        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
        if (reader == nullptr)
            return nullptr;

        auto* ptr = reader.get();
        cache[key] = std::move (reader);
        return ptr;
    }

    void clear() { cache.clear(); }

private:
    juce::AudioFormatManager& formatManager;
    std::map<std::string, std::unique_ptr<juce::AudioFormatReader>> cache;
};
