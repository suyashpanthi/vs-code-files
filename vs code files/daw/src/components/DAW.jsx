import { useRef, useEffect, useCallback } from 'react';
import useDAWState from '../hooks/useDAWState';
import audioEngine from '../audio/AudioEngine';
import Toolbar from './Toolbar';
import Transport from './Transport';
import Timeline from './Timeline';
import Mixer from './Mixer';
import YouTubeModal from './YouTubeModal';
import { useState } from 'react';
import './DAW.css';

export default function DAW() {
  const state = useDAWState();
  const animFrameRef = useRef(null);
  const [youtubeModalOpen, setYoutubeModalOpen] = useState(false);

  // Initialize audio engine track nodes when tracks change
  useEffect(() => {
    state.tracks.forEach(track => {
      if (!audioEngine.trackNodes.has(track.id)) {
        audioEngine.createTrackNodes(track.id);
      }
    });
  }, [state.tracks]);

  // Sync track parameters to audio engine
  useEffect(() => {
    state.tracks.forEach(track => {
      const hasSolo = state.tracks.some(t => t.solo);
      const isMuted = track.muted || (hasSolo && !track.solo);
      if (isMuted) {
        audioEngine.setTrackVolume(track.id, 0);
      } else {
        audioEngine.setTrackVolume(track.id, track.volume);
      }
      audioEngine.setTrackPan(track.id, track.pan);
      audioEngine.setTrackEQ(track.id, 'low', track.eqLow);
      audioEngine.setTrackEQ(track.id, 'mid', track.eqMid);
      audioEngine.setTrackEQ(track.id, 'high', track.eqHigh);
    });
  }, [state.tracks]);

  // Playback animation loop
  useEffect(() => {
    if (state.transport.playing) {
      const tick = () => {
        const currentTime = audioEngine.getCurrentTime();
        state.setPosition(currentTime);

        // Check if we've exceeded total duration
        const totalDuration = state.getTotalDuration();
        if (currentTime >= totalDuration - 5) {
          if (state.transport.loop) {
            handlePlay(state.transport.loopStart);
          } else {
            handleStop();
            return;
          }
        }

        animFrameRef.current = requestAnimationFrame(tick);
      };
      animFrameRef.current = requestAnimationFrame(tick);
    }
    return () => {
      if (animFrameRef.current) cancelAnimationFrame(animFrameRef.current);
    };
  }, [state.transport.playing]);

  const handlePlay = useCallback((fromPosition) => {
    audioEngine.ensureContext();
    const pos = fromPosition !== undefined ? fromPosition : state.transport.position;
    audioEngine.playTracks(state.tracks, pos);
    state.setPlaying(true);
  }, [state.tracks, state.transport.position]);

  const handleStop = useCallback(() => {
    audioEngine.stopAll();
    state.setPlaying(false);
  }, []);

  const handleStopAndReset = useCallback(() => {
    audioEngine.stopAll();
    state.setPlaying(false);
    state.setPosition(0);
  }, []);

  const handleRecord = useCallback(async () => {
    if (state.transport.recording) {
      audioEngine.stopRecording();
      state.setRecording(false);
      return;
    }

    let trackId = state.selectedTrackId;
    if (!trackId) {
      trackId = state.addTrack('Recording');
    }

    state.setRecording(true);
    const recordPosition = state.transport.position;

    try {
      const recordPromise = audioEngine.startRecording();
      // Also start playback while recording
      handlePlay(recordPosition);

      // Wait for recording to finish (user clicks record again)
      const audioBuffer = await recordPromise;
      state.addRegion(trackId, audioBuffer, recordPosition, 'Recorded');
    } catch (err) {
      console.error('Recording failed:', err);
      state.setRecording(false);
    }
  }, [state.selectedTrackId, state.transport.position, state.transport.recording]);

  const handleImportFile = useCallback(async (file, trackId) => {
    audioEngine.ensureContext();
    try {
      const audioBuffer = await audioEngine.loadFile(file);
      let tId = trackId || state.selectedTrackId;
      if (!tId) {
        tId = state.addTrack(file.name.replace(/\.[^.]+$/, ''));
      }
      state.addRegion(tId, audioBuffer, 0, file.name);
    } catch (err) {
      console.error('Failed to import file:', err);
    }
  }, [state.selectedTrackId]);

  const handleYoutubeDownload = useCallback(async (url) => {
    try {
      const response = await fetch('http://localhost:5000/api/download', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ url }),
      });

      if (!response.ok) {
        const err = await response.json();
        throw new Error(err.error || 'Download failed');
      }

      const blob = await response.blob();
      const arrayBuffer = await blob.arrayBuffer();
      audioEngine.ensureContext();
      const audioBuffer = await audioEngine.decodeAudioData(arrayBuffer);
      const trackId = state.addTrack('YouTube Import');
      state.addRegion(trackId, audioBuffer, 0, 'YouTube Audio');
      setYoutubeModalOpen(false);
    } catch (err) {
      throw err;
    }
  }, []);

  const handleExport = useCallback(async () => {
    const duration = state.getTotalDuration() - 5;
    if (duration <= 0) return;

    try {
      const renderedBuffer = await audioEngine.mixDown(state.tracks, duration);
      const wavBlob = audioEngine.audioBufferToWav(renderedBuffer);
      const url = URL.createObjectURL(wavBlob);
      const a = document.createElement('a');
      a.href = url;
      a.download = 'mix.wav';
      a.click();
      URL.revokeObjectURL(url);
    } catch (err) {
      console.error('Export failed:', err);
    }
  }, [state.tracks]);

  const handleAddTrack = useCallback(() => {
    state.addTrack();
  }, []);

  const handleRemoveTrack = useCallback((trackId) => {
    audioEngine.removeTrackNodes(trackId);
    state.removeTrack(trackId);
  }, []);

  return (
    <div className="daw">
      <Toolbar
        onAddTrack={handleAddTrack}
        onImportFile={handleImportFile}
        onOpenYoutube={() => setYoutubeModalOpen(true)}
        onExport={handleExport}
        mixerVisible={state.mixerVisible}
        onToggleMixer={() => state.setMixerVisible(!state.mixerVisible)}
      />
      <Transport
        transport={state.transport}
        onPlay={() => handlePlay()}
        onStop={handleStopAndReset}
        onPause={handleStop}
        onRecord={handleRecord}
        onToggleLoop={state.toggleLoop}
        onBpmChange={state.setBpm}
        onPositionChange={state.setPosition}
      />
      <Timeline
        tracks={state.tracks}
        transport={state.transport}
        zoom={state.zoom}
        onZoomChange={state.setZoom}
        selectedTrackId={state.selectedTrackId}
        selectedRegionId={state.selectedRegionId}
        onSelectTrack={state.setSelectedTrackId}
        onSelectRegion={state.setSelectedRegionId}
        onUpdateTrack={state.updateTrack}
        onRemoveTrack={handleRemoveTrack}
        onPositionChange={state.setPosition}
        onSplitRegion={state.splitRegion}
        onRemoveRegion={state.removeRegion}
        onMoveRegion={state.moveRegion}
        onImportFile={handleImportFile}
        onAddRegion={state.addRegion}
        getTotalDuration={state.getTotalDuration}
      />
      {state.mixerVisible && (
        <Mixer
          tracks={state.tracks}
          onUpdateTrack={state.updateTrack}
          selectedTrackId={state.selectedTrackId}
          onSelectTrack={state.setSelectedTrackId}
        />
      )}
      {youtubeModalOpen && (
        <YouTubeModal
          onDownload={handleYoutubeDownload}
          onClose={() => setYoutubeModalOpen(false)}
        />
      )}
    </div>
  );
}
