#include "Transport.h"

Transport::Transport() {}

void Transport::play()
{
    state.store (TransportState::Playing);
    listeners.call ([](Listener& l) { l.transportStateChanged (TransportState::Playing); });
}

void Transport::stop()
{
    state.store (TransportState::Stopped);
    positionInSamples.store (0);
    listeners.call ([](Listener& l) { l.transportStateChanged (TransportState::Stopped); });
}

void Transport::pause()
{
    if (isPlaying())
    {
        state.store (TransportState::Paused);
        listeners.call ([](Listener& l) { l.transportStateChanged (TransportState::Paused); });
    }
}

void Transport::togglePlayStop()
{
    if (isPlaying())
        stop();
    else
        play();
}

void Transport::returnToStart()
{
    positionInSamples.store (0);
}

void Transport::advancePosition (int numSamples)
{
    if (! isPlaying())
        return;

    auto pos = positionInSamples.load() + numSamples;

    if (loopEnabled.load())
    {
        auto lEnd = loopEnd.load();
        auto lStart = loopStart.load();
        if (lEnd > lStart && pos >= lEnd)
            pos = lStart + (pos - lEnd);
    }

    positionInSamples.store (pos);
}

void Transport::setLoopRange (int64_t startSample, int64_t endSample)
{
    loopStart.store (startSample);
    loopEnd.store (endSample);
}
