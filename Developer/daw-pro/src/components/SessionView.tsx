import React, { useState, useCallback, useRef, useEffect, useMemo } from 'react';
import { useDAWStore } from '../store';
import type { Track, Clip, Scene } from '../types';
import './SessionView.css';

interface ContextMenuState {
  visible: boolean;
  x: number;
  y: number;
  trackId: string;
  sceneIndex: number;
}

const ClipSlot: React.FC<{
  track: Track;
  sceneIndex: number;
  clip: Clip | undefined;
  isSelected: boolean;
  onLaunch: (trackId: string, sceneIndex: number) => void;
  onCreate: (trackId: string, sceneIndex: number) => void;
  onSelect: (clipId: string | null) => void;
  onContextMenu: (e: React.MouseEvent, trackId: string, sceneIndex: number) => void;
}> = React.memo(({ track, sceneIndex, clip, isSelected, onLaunch, onCreate, onSelect, onContextMenu }) => {
  const handleClick = useCallback(() => {
    if (clip) {
      onSelect(clip.id);
      onLaunch(track.id, sceneIndex);
    } else {
      onCreate(track.id, sceneIndex);
    }
  }, [clip, track.id, sceneIndex, onLaunch, onCreate, onSelect]);

  const handleContextMenu = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    if (clip) {
      onContextMenu(e, track.id, sceneIndex);
    }
  }, [clip, track.id, sceneIndex, onContextMenu]);

  return (
    <div
      className={`session-clip-slot ${clip ? 'has-clip' : 'empty'} ${isSelected ? 'selected' : ''} ${clip?.isPlaying ? 'playing' : ''}`}
      onClick={handleClick}
      onContextMenu={handleContextMenu}
      style={clip ? { backgroundColor: clip.color + '88' } : undefined}
    >
      {clip && (
        <>
          <span className="clip-slot-name">{clip.name}</span>
          {clip.isPlaying && (
            <span className="clip-playing-indicator">
              <svg width="10" height="10" viewBox="0 0 10 10">
                <polygon points="1,0 10,5 1,10" fill="#00ff88" />
              </svg>
            </span>
          )}
        </>
      )}
      {!clip && (
        <span className="clip-slot-empty-icon">+</span>
      )}
    </div>
  );
});

const TrackHeader: React.FC<{
  track: Track;
  isSelected: boolean;
  onSelect: (id: string) => void;
  onToggleMute: (id: string) => void;
  onToggleSolo: (id: string) => void;
  onToggleArm: (id: string) => void;
  onStopClip: (id: string) => void;
}> = React.memo(({ track, isSelected, onSelect, onToggleMute, onToggleSolo, onToggleArm, onStopClip }) => {
  return (
    <div
      className={`session-track-header ${isSelected ? 'selected' : ''}`}
      onClick={() => onSelect(track.id)}
    >
      <div className="track-header-color-strip" style={{ backgroundColor: track.color }} />
      <div className="track-header-info">
        <span className="track-header-name">{track.name}</span>
        <span className="track-header-type">{track.type}</span>
      </div>
      <div className="track-header-buttons">
        <button
          className={`track-btn arm-btn ${track.armed ? 'active' : ''}`}
          onClick={(e) => { e.stopPropagation(); onToggleArm(track.id); }}
          title="Arm"
        >
          <svg width="10" height="10" viewBox="0 0 10 10">
            <circle cx="5" cy="5" r="4" fill="none" stroke="currentColor" strokeWidth="1.5" />
          </svg>
        </button>
        <button
          className={`track-btn solo-btn ${track.solo ? 'active' : ''}`}
          onClick={(e) => { e.stopPropagation(); onToggleSolo(track.id); }}
          title="Solo"
        >
          S
        </button>
        <button
          className={`track-btn mute-btn ${track.mute ? 'active' : ''}`}
          onClick={(e) => { e.stopPropagation(); onToggleMute(track.id); }}
          title="Mute"
        >
          M
        </button>
      </div>
      <button
        className="track-clip-stop-btn"
        onClick={(e) => { e.stopPropagation(); onStopClip(track.id); }}
        title="Stop Clip"
      >
        <svg width="8" height="8" viewBox="0 0 8 8">
          <rect x="0" y="0" width="8" height="8" fill="currentColor" />
        </svg>
      </button>
    </div>
  );
});

const SceneLauncher: React.FC<{
  scene: Scene;
  sceneIndex: number;
  isSelected: boolean;
  onLaunch: (index: number) => void;
  onSelect: (index: number) => void;
}> = React.memo(({ scene, sceneIndex, isSelected, onLaunch, onSelect }) => {
  return (
    <div
      className={`session-scene-launcher ${isSelected ? 'selected' : ''}`}
      onClick={() => onSelect(sceneIndex)}
    >
      <span className="scene-number">{sceneIndex + 1}</span>
      <span className="scene-name">{scene.name}</span>
      <button
        className="scene-launch-btn"
        onClick={(e) => { e.stopPropagation(); onLaunch(sceneIndex); }}
        title={`Launch Scene ${sceneIndex + 1}`}
      >
        <svg width="12" height="12" viewBox="0 0 12 12">
          <polygon points="1,0 12,6 1,12" fill="currentColor" />
        </svg>
      </button>
    </div>
  );
});

const MasterColumnCell: React.FC<{
  sceneIndex: number;
  masterTrack: Track;
}> = React.memo(({ sceneIndex, masterTrack }) => {
  const clip = masterTrack.clips[sceneIndex];
  return (
    <div className="session-master-cell">
      {clip ? (
        <span className="master-cell-clip" style={{ backgroundColor: clip.color + '88' }}>
          {clip.name}
        </span>
      ) : null}
    </div>
  );
});

const SessionView: React.FC = () => {
  const tracks = useDAWStore((s) => s.tracks);
  const scenes = useDAWStore((s) => s.scenes);
  const masterTrack = useDAWStore((s) => s.masterTrack);
  const returnTracks = useDAWStore((s) => s.returnTracks);
  const selectedTrackId = useDAWStore((s) => s.selectedTrackId);
  const selectedSceneIndex = useDAWStore((s) => s.selectedSceneIndex);
  const selectedClipId = useDAWStore((s) => s.selectedClipId);

  const selectTrack = useDAWStore((s) => s.selectTrack);
  const selectScene = useDAWStore((s) => s.selectScene);
  const selectClip = useDAWStore((s) => s.selectClip);
  const launchClip = useDAWStore((s) => s.launchClip);
  const stopTrackClip = useDAWStore((s) => s.stopTrackClip);
  const launchScene = useDAWStore((s) => s.launchScene);
  const stopAllClips = useDAWStore((s) => s.stopAllClips);
  const createClip = useDAWStore((s) => s.createClip);
  const deleteClip = useDAWStore((s) => s.deleteClip);
  const toggleTrackMute = useDAWStore((s) => s.toggleTrackMute);
  const toggleTrackSolo = useDAWStore((s) => s.toggleTrackSolo);
  const toggleTrackArm = useDAWStore((s) => s.toggleTrackArm);
  const updateClip = useDAWStore((s) => s.updateClip);

  const gridRef = useRef<HTMLDivElement>(null);
  const [contextMenu, setContextMenu] = useState<ContextMenuState>({
    visible: false,
    x: 0,
    y: 0,
    trackId: '',
    sceneIndex: 0,
  });

  const allDisplayTracks = useMemo(() => {
    return [...tracks, ...returnTracks];
  }, [tracks, returnTracks]);

  const handleContextMenu = useCallback((e: React.MouseEvent, trackId: string, sceneIndex: number) => {
    setContextMenu({
      visible: true,
      x: e.clientX,
      y: e.clientY,
      trackId,
      sceneIndex,
    });
  }, []);

  const closeContextMenu = useCallback(() => {
    setContextMenu((prev) => ({ ...prev, visible: false }));
  }, []);

  const handleDeleteClip = useCallback(() => {
    deleteClip(contextMenu.trackId, contextMenu.sceneIndex);
    closeContextMenu();
  }, [contextMenu.trackId, contextMenu.sceneIndex, deleteClip, closeContextMenu]);

  const handleDuplicateClip = useCallback(() => {
    const track = allDisplayTracks.find((t) => t.id === contextMenu.trackId);
    if (!track) return;
    const clip = track.clips[contextMenu.sceneIndex];
    if (!clip) return;
    // Find the next empty scene index for this track
    let nextIndex = contextMenu.sceneIndex + 1;
    while (track.clips[nextIndex] && nextIndex < scenes.length) {
      nextIndex++;
    }
    if (nextIndex < scenes.length) {
      createClip(contextMenu.trackId, nextIndex);
      // Copy clip properties from original
      const newTrack = useDAWStore.getState().tracks.find((t) => t.id === contextMenu.trackId);
      if (newTrack && newTrack.clips[nextIndex]) {
        updateClip(newTrack.clips[nextIndex].id, {
          name: clip.name,
          color: clip.color,
          notes: clip.notes ? [...clip.notes] : undefined,
          loopEnabled: clip.loopEnabled,
          loopStart: clip.loopStart,
          loopEnd: clip.loopEnd,
        });
      }
    }
    closeContextMenu();
  }, [contextMenu, allDisplayTracks, scenes.length, createClip, updateClip, closeContextMenu]);

  const handleRenameClip = useCallback(() => {
    const track = allDisplayTracks.find((t) => t.id === contextMenu.trackId);
    if (!track) return;
    const clip = track.clips[contextMenu.sceneIndex];
    if (!clip) return;
    const newName = window.prompt('Rename clip:', clip.name);
    if (newName !== null && newName.trim() !== '') {
      updateClip(clip.id, { name: newName.trim() });
    }
    closeContextMenu();
  }, [contextMenu, allDisplayTracks, updateClip, closeContextMenu]);

  // Close context menu on click outside
  useEffect(() => {
    if (contextMenu.visible) {
      const handler = () => closeContextMenu();
      window.addEventListener('click', handler);
      return () => window.removeEventListener('click', handler);
    }
  }, [contextMenu.visible, closeContextMenu]);

  return (
    <div className="session-view">
      <div className="session-grid-container" ref={gridRef}>
        {/* Track Headers Row */}
        <div className="session-headers-row">
          <div className="session-scene-header-spacer" />
          {allDisplayTracks.map((track) => (
            <TrackHeader
              key={track.id}
              track={track}
              isSelected={selectedTrackId === track.id}
              onSelect={selectTrack}
              onToggleMute={toggleTrackMute}
              onToggleSolo={toggleTrackSolo}
              onToggleArm={toggleTrackArm}
              onStopClip={stopTrackClip}
            />
          ))}
          {/* Master header */}
          <div className="session-master-header">
            <div className="track-header-color-strip" style={{ backgroundColor: masterTrack.color }} />
            <span className="track-header-name">Master</span>
          </div>
        </div>

        {/* Clip Grid */}
        <div className="session-grid-scroll">
          {scenes.map((scene, sceneIndex) => (
            <div
              key={scene.id}
              className={`session-grid-row ${selectedSceneIndex === sceneIndex ? 'selected-scene' : ''}`}
            >
              {/* Scene info (left spacer area) */}
              <div className="session-row-scene-info" onClick={() => selectScene(sceneIndex)}>
                <span className="session-row-scene-number">{sceneIndex + 1}</span>
              </div>

              {/* Clip slots for each track */}
              {allDisplayTracks.map((track) => (
                <ClipSlot
                  key={`${track.id}-${sceneIndex}`}
                  track={track}
                  sceneIndex={sceneIndex}
                  clip={track.clips[sceneIndex]}
                  isSelected={track.clips[sceneIndex]?.id === selectedClipId}
                  onLaunch={launchClip}
                  onCreate={createClip}
                  onSelect={selectClip}
                  onContextMenu={handleContextMenu}
                />
              ))}

              {/* Master column cell */}
              <MasterColumnCell sceneIndex={sceneIndex} masterTrack={masterTrack} />

              {/* Scene launcher on the right */}
              <SceneLauncher
                scene={scene}
                sceneIndex={sceneIndex}
                isSelected={selectedSceneIndex === sceneIndex}
                onLaunch={launchScene}
                onSelect={selectScene}
              />
            </div>
          ))}

          {/* Stop all row */}
          <div className="session-stop-all-row">
            <div className="session-row-scene-info" />
            {allDisplayTracks.map((track) => (
              <div key={track.id} className="session-stop-track-cell">
                <button
                  className="stop-track-btn"
                  onClick={() => stopTrackClip(track.id)}
                  title={`Stop ${track.name}`}
                >
                  <svg width="8" height="8" viewBox="0 0 8 8">
                    <rect x="0" y="0" width="8" height="8" fill="currentColor" />
                  </svg>
                </button>
              </div>
            ))}
            <div className="session-master-cell" />
            <div className="session-stop-all-launcher">
              <button
                className="stop-all-btn"
                onClick={stopAllClips}
                title="Stop All Clips"
              >
                <svg width="12" height="12" viewBox="0 0 12 12">
                  <rect x="1" y="1" width="10" height="10" fill="currentColor" />
                </svg>
              </button>
            </div>
          </div>
        </div>
      </div>

      {/* Context Menu */}
      {contextMenu.visible && (
        <div
          className="session-context-menu"
          style={{ left: contextMenu.x, top: contextMenu.y }}
          onClick={(e) => e.stopPropagation()}
        >
          <button className="context-menu-item" onClick={handleRenameClip}>
            Rename
          </button>
          <button className="context-menu-item" onClick={handleDuplicateClip}>
            Duplicate
          </button>
          <div className="context-menu-divider" />
          <button className="context-menu-item danger" onClick={handleDeleteClip}>
            Delete
          </button>
        </div>
      )}
    </div>
  );
};

export default SessionView;
