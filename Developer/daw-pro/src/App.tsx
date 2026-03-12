import React, { useEffect, useCallback, useRef } from 'react';
import { useDAWStore } from './store';
import { audioEngine } from './engine/AudioEngine';
import SessionView from './components/SessionView';
import ArrangementView from './components/ArrangementView';
import MixerView from './components/MixerView';
import PianoRoll from './components/PianoRoll';
import Transport from './components/Transport';
import Browser from './components/Browser';
import DeviceChain from './components/DeviceChain';
import './App.css';

export default function App() {
  const activeView = useDAWStore((s) => s.activeView);
  const showBrowser = useDAWStore((s) => s.showBrowser);
  const showDetail = useDAWStore((s) => s.showDetail);
  const showMixer = useDAWStore((s) => s.showMixer);
  const detailView = useDAWStore((s) => s.detailView);
  const selectedClipId = useDAWStore((s) => s.selectedClipId);
  const tracks = useDAWStore((s) => s.tracks);
  const returnTracks = useDAWStore((s) => s.returnTracks);
  const toggleView = useDAWStore((s) => s.toggleView);
  const toggleBrowser = useDAWStore((s) => s.toggleBrowser);
  const toggleDetail = useDAWStore((s) => s.toggleDetail);
  const toggleMixer = useDAWStore((s) => s.toggleMixer);
  const togglePlay = useDAWStore((s) => s.togglePlay);
  const stop = useDAWStore((s) => s.stop);
  const record = useDAWStore((s) => s.record);
  const toggleLoop = useDAWStore((s) => s.toggleLoop);
  const toggleMetronome = useDAWStore((s) => s.toggleMetronome);
  const setTrackMeter = useDAWStore((s) => s.setTrackMeter);
  const setMasterMeter = useDAWStore((s) => s.setMasterMeter);
  const addTrack = useDAWStore((s) => s.addTrack);
  const bpm = useDAWStore((s) => s.bpm);
  const initialized = useRef(false);

  // Initialize audio engine
  useEffect(() => {
    if (initialized.current) return;
    initialized.current = true;

    const initEngine = async () => {
      await audioEngine.init();

      // Create engine tracks for all existing tracks
      const state = useDAWStore.getState();
      state.tracks.forEach((t) => audioEngine.createTrack(t.id, t.type));
      state.returnTracks.forEach((t) => audioEngine.createTrack(t.id, 'return'));

      // Set up metering callbacks
      audioEngine.onMeter((trackId, left, right) => {
        useDAWStore.getState().setTrackMeter(trackId, left, right);
      });
      audioEngine.onMasterMeter((left, right) => {
        useDAWStore.getState().setMasterMeter(left, right);
      });
    };

    initEngine();

    return () => {
      // Don't dispose on StrictMode remount
    };
  }, []);

  // Sync BPM
  useEffect(() => {
    audioEngine.setTempo(bpm);
  }, [bpm]);

  // Keyboard shortcuts
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Don't capture when typing in inputs
      if (
        e.target instanceof HTMLInputElement ||
        e.target instanceof HTMLTextAreaElement ||
        (e.target as HTMLElement)?.isContentEditable
      ) return;

      const ctrl = e.metaKey || e.ctrlKey;

      switch (e.key) {
        case ' ':
          e.preventDefault();
          togglePlay();
          break;
        case 'Tab':
          e.preventDefault();
          toggleView();
          break;
        case 'b':
          if (ctrl && e.altKey) { e.preventDefault(); toggleBrowser(); }
          else if (!ctrl) { e.preventDefault(); useDAWStore.getState().toggleDrawMode(); }
          break;
        case 'l':
          if (ctrl) { e.preventDefault(); toggleLoop(); }
          break;
        case 'm':
          if (ctrl) { e.preventDefault(); toggleMetronome(); }
          break;
        case 'r':
          if (ctrl && e.shiftKey) { e.preventDefault(); /* export */ }
          break;
        case 't':
          if (ctrl && e.shiftKey) { e.preventDefault(); addTrack('midi'); }
          else if (ctrl) { e.preventDefault(); addTrack('audio'); }
          break;
        case 'Escape':
          stop();
          break;
        case 'Home':
          e.preventDefault();
          useDAWStore.getState().setCurrentBeat(0);
          audioEngine.setPosition(0);
          break;
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [togglePlay, toggleView, toggleBrowser, toggleLoop, toggleMetronome, stop, addTrack]);

  // Find selected clip for piano roll
  const selectedClip = React.useMemo(() => {
    if (!selectedClipId) return null;
    for (const track of tracks) {
      for (const clip of Object.values(track.clips)) {
        if (clip.id === selectedClipId) return clip;
      }
      for (const clip of track.arrangementClips) {
        if (clip.id === selectedClipId) return clip;
      }
    }
    return null;
  }, [selectedClipId, tracks]);

  const showPianoRoll = showDetail && detailView === 'clip' && selectedClip?.type === 'midi';
  const showDeviceChain = showDetail && (detailView === 'device' || !showPianoRoll);

  return (
    <div className="daw-app">
      {/* Top title bar */}
      <div className="daw-title-bar">
        <div className="daw-title-left">
          <span className="daw-logo">DAW Pro</span>
          <div className="daw-title-tabs">
            <button
              className={`daw-tab ${activeView === 'session' ? 'active' : ''}`}
              onClick={() => useDAWStore.getState().setActiveView('session')}
            >
              Session
            </button>
            <button
              className={`daw-tab ${activeView === 'arrangement' ? 'active' : ''}`}
              onClick={() => useDAWStore.getState().setActiveView('arrangement')}
            >
              Arrangement
            </button>
          </div>
        </div>
        <div className="daw-title-center">
          <span className="daw-project-name">
            {useDAWStore.getState().projectName}
          </span>
        </div>
        <div className="daw-title-right">
          <button
            className={`daw-panel-toggle ${showBrowser ? 'active' : ''}`}
            onClick={toggleBrowser}
            title="Toggle Browser (Ctrl+Alt+B)"
          >
            ☰
          </button>
          <button
            className={`daw-panel-toggle ${showDetail ? 'active' : ''}`}
            onClick={toggleDetail}
            title="Toggle Detail"
          >
            ⬒
          </button>
          <button
            className={`daw-panel-toggle ${showMixer ? 'active' : ''}`}
            onClick={toggleMixer}
            title="Toggle Mixer"
          >
            ⊞
          </button>
        </div>
      </div>

      {/* Main content area */}
      <div className="daw-main">
        {/* Browser sidebar */}
        {showBrowser && (
          <div className="daw-browser-panel">
            <Browser />
          </div>
        )}

        {/* Center: main view + detail */}
        <div className="daw-center">
          {/* Main view */}
          <div className="daw-main-view">
            {activeView === 'session' ? <SessionView /> : <ArrangementView />}
          </div>

          {/* Detail panel */}
          {showDetail && (
            <div className="daw-detail-panel">
              <div className="daw-detail-tabs">
                <button
                  className={`daw-detail-tab ${detailView === 'clip' ? 'active' : ''}`}
                  onClick={() => useDAWStore.getState().setDetailView('clip')}
                >
                  Clip
                </button>
                <button
                  className={`daw-detail-tab ${detailView === 'device' ? 'active' : ''}`}
                  onClick={() => useDAWStore.getState().setDetailView('device')}
                >
                  Device
                </button>
              </div>
              <div className="daw-detail-content">
                {showPianoRoll ? <PianoRoll /> : <DeviceChain />}
              </div>
            </div>
          )}

          {/* Mixer */}
          {showMixer && (
            <div className="daw-mixer-panel">
              <MixerView />
            </div>
          )}
        </div>
      </div>

      {/* Transport bar */}
      <Transport />

      {/* Audio context resume overlay */}
      <AudioContextOverlay />
    </div>
  );
}

function AudioContextOverlay() {
  const [needsResume, setNeedsResume] = React.useState(true);

  const handleClick = async () => {
    await audioEngine.init();
    setNeedsResume(false);

    // Create engine tracks
    const state = useDAWStore.getState();
    state.tracks.forEach((t) => {
      audioEngine.createTrack(t.id, t.type);
    });
    state.returnTracks.forEach((t) => {
      audioEngine.createTrack(t.id, 'return');
    });

    audioEngine.onMeter((trackId, left, right) => {
      useDAWStore.getState().setTrackMeter(trackId, left, right);
    });
    audioEngine.onMasterMeter((left, right) => {
      useDAWStore.getState().setMasterMeter(left, right);
    });
  };

  if (!needsResume) return null;

  return (
    <div className="audio-overlay" onClick={handleClick}>
      <div className="audio-overlay-content">
        <div className="audio-overlay-icon">🎵</div>
        <h2>DAW Pro</h2>
        <p>Click anywhere to start the audio engine</p>
        <div className="audio-overlay-hint">
          Professional Digital Audio Workstation
        </div>
      </div>
    </div>
  );
}
