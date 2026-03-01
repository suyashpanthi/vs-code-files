#pragma once
#include <JuceHeader.h>
#include "TransportState.h"
#include "TempoMap.h"

class Transport
{
public:
    Transport();

    void play();
    void stop();
    void pause();
    void togglePlayStop();
    void returnToStart();

    TransportState getState() const { return state.load(); }
    bool isPlaying() const   { return state.load() == TransportState::Playing; }
    bool isPaused() const    { return state.load() == TransportState::Paused; }
    bool isStopped() const   { return state.load() == TransportState::Stopped; }
    bool isRecording() const { return state.load() == TransportState::Recording; }

    int64_t getPositionInSamples() const { return positionInSamples.load(); }
    void setPositionInSamples (int64_t pos) { positionInSamples.store (pos); }

    void advancePosition (int numSamples);

    // Loop
    void setLoopEnabled (bool enabled) { loopEnabled.store (enabled); }
    bool isLoopEnabled() const { return loopEnabled.load(); }

    void setLoopRange (int64_t startSample, int64_t endSample);
    int64_t getLoopStart() const { return loopStart.load(); }
    int64_t getLoopEnd() const   { return loopEnd.load(); }

    TempoMap& getTempoMap() { return tempoMap; }
    const TempoMap& getTempoMap() const { return tempoMap; }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void transportStateChanged (TransportState newState) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    std::atomic<TransportState> state { TransportState::Stopped };
    std::atomic<int64_t> positionInSamples { 0 };

    std::atomic<bool> loopEnabled { false };
    std::atomic<int64_t> loopStart { 0 };
    std::atomic<int64_t> loopEnd { 0 };

    TempoMap tempoMap;
    juce::ListenerList<Listener> listeners;
};
