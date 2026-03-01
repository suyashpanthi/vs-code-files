#include "BounceExporter.h"
#include "../Engine/MasterBusSource.h"
#include "../Engine/TrackAudioSource.h"

BounceExporter::BounceExporter (AudioEngine& e) : engine (e) {}

void BounceExporter::bounceDialog (Project& project)
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Bounce to File",
        juce::File(),
        "*.wav");

    chooser->launchAsync (juce::FileBrowserComponent::saveMode,
        [this, &project, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
                bounceToFile (project, file.withFileExtension ("wav"));
        });
}

bool BounceExporter::bounceToFile (Project& project, const juce::File& outputFile,
                                    double sampleRate, int bitsPerSample)
{
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer (
        wavFormat.createWriterFor (new juce::FileOutputStream (outputFile),
                                   sampleRate, 2, bitsPerSample, {}, 0));

    if (writer == nullptr)
        return false;

    // Find the total length to render
    int64_t totalLength = 0;
    for (auto* track : project.getTracks())
    {
        for (auto* region : track->getRegions())
        {
            int64_t regionEnd = region->getPositionInSamples() + region->getLengthInSamples();
            if (regionEnd > totalLength)
                totalLength = regionEnd;
        }
    }

    if (totalLength == 0)
        return false;

    // Render offline using the master bus
    auto& masterBus = engine.getMasterBus();
    auto& transport = engine.getTransport();

    transport.stop();
    transport.setPositionInSamples (0);
    masterBus.prepareToPlay (1024, sampleRate);
    transport.play();

    juce::AudioBuffer<float> buffer (2, 1024);
    int64_t samplesWritten = 0;

    while (samplesWritten < totalLength)
    {
        int blockSize = (int) juce::jmin ((int64_t) 1024, totalLength - samplesWritten);
        buffer.clear();

        juce::AudioSourceChannelInfo info (&buffer, 0, blockSize);
        masterBus.getNextAudioBlock (info);

        writer->writeFromAudioSampleBuffer (buffer, 0, blockSize);
        samplesWritten += blockSize;
    }

    transport.stop();
    masterBus.releaseResources();

    return true;
}
