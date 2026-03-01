#include "WaveformView.h"
#include "../Colours.h"

WaveformView::WaveformView (juce::AudioThumbnailCache& cache, juce::AudioFormatManager& fm)
    : thumbCache (cache), formatMgr (fm)
{
}

void WaveformView::setRegion (AudioRegion* region)
{
    currentRegion = region;

    if (region != nullptr && region->getFormatReader() != nullptr)
    {
        thumbnail = std::make_unique<juce::AudioThumbnail> (512, formatMgr, thumbCache);
        thumbnail->setReader (region->getFormatReader(), (juce::int64) region->getFile().hashCode64());
    }
    else
    {
        thumbnail = nullptr;
    }

    repaint();
}

void WaveformView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (DAWColours::regionBackground);
    g.fillRoundedRectangle (bounds, 3.0f);

    if (thumbnail != nullptr && thumbnail->getTotalLength() > 0.0)
    {
        g.setColour (waveformColour.withAlpha (0.7f));
        thumbnail->drawChannels (g, getLocalBounds().reduced (2, 4),
                                 0.0, thumbnail->getTotalLength(), 1.0f);

        g.setColour (waveformColour.withAlpha (0.3f));
        thumbnail->drawChannels (g, getLocalBounds().reduced (2, 4),
                                 0.0, thumbnail->getTotalLength(), 0.5f);
    }

    // Border
    g.setColour (waveformColour.withAlpha (0.5f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);

    // Region name
    if (currentRegion != nullptr)
    {
        g.setColour (DAWColours::textPrimary);
        g.setFont (11.0f);
        g.drawText (currentRegion->getName(), getLocalBounds().reduced (6, 2),
                    juce::Justification::topLeft, true);
    }
}

void WaveformView::resized() {}
