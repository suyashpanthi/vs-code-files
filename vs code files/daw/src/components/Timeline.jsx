import { useRef, useCallback, useEffect } from 'react';
import Track from './Track';
import './Timeline.css';

export default function Timeline({
  tracks, transport, zoom, onZoomChange,
  selectedTrackId, selectedRegionId,
  onSelectTrack, onSelectRegion,
  onUpdateTrack, onRemoveTrack,
  onPositionChange, onSplitRegion, onRemoveRegion,
  onMoveRegion, onImportFile, onAddRegion,
  getTotalDuration
}) {
  const timelineRef = useRef(null);
  const rulerRef = useRef(null);
  const cursorRef = useRef(null);

  const totalDuration = getTotalDuration();
  const totalWidth = totalDuration * zoom;
  const beatsPerSecond = transport.bpm / 60;
  const pixelsPerBeat = zoom / beatsPerSecond;

  // Auto-scroll to follow cursor during playback
  useEffect(() => {
    if (transport.playing && timelineRef.current) {
      const cursorX = transport.position * zoom;
      const container = timelineRef.current;
      const scrollLeft = container.scrollLeft;
      const visibleWidth = container.clientWidth - 200; // minus track header
      if (cursorX > scrollLeft + visibleWidth - 100 || cursorX < scrollLeft) {
        container.scrollLeft = cursorX - 100;
      }
    }
  }, [transport.position, transport.playing, zoom]);

  const handleRulerClick = useCallback((e) => {
    const rect = e.currentTarget.getBoundingClientRect();
    const x = e.clientX - rect.left + (timelineRef.current?.scrollLeft || 0);
    const time = x / zoom;
    onPositionChange(Math.max(0, time));
  }, [zoom, onPositionChange]);

  const handleWheel = useCallback((e) => {
    if (e.ctrlKey || e.metaKey) {
      e.preventDefault();
      const delta = e.deltaY > 0 ? -10 : 10;
      onZoomChange(Math.max(20, Math.min(500, zoom + delta)));
    }
  }, [zoom, onZoomChange]);

  const handleDrop = useCallback((e) => {
    e.preventDefault();
    const files = e.dataTransfer.files;
    if (files.length > 0 && files[0].type.startsWith('audio/')) {
      onImportFile(files[0]);
    }
  }, [onImportFile]);

  const handleDragOver = useCallback((e) => {
    e.preventDefault();
    e.dataTransfer.dropEffect = 'copy';
  }, []);

  // Generate ruler marks
  const rulerMarks = [];
  const step = pixelsPerBeat >= 20 ? 1 : pixelsPerBeat >= 10 ? 2 : 4;
  const totalBeats = totalDuration * beatsPerSecond;
  for (let beat = 0; beat <= totalBeats; beat += step) {
    const x = (beat / beatsPerSecond) * zoom;
    const isMeasure = beat % 4 === 0;
    const measureNum = Math.floor(beat / 4) + 1;
    rulerMarks.push(
      <div
        key={beat}
        className={`ruler-mark ${isMeasure ? 'measure' : 'beat'}`}
        style={{ left: x }}
      >
        {isMeasure && <span className="ruler-label">{measureNum}</span>}
      </div>
    );
  }

  const cursorX = transport.position * zoom;

  return (
    <div
      className="timeline-container"
      ref={timelineRef}
      onWheel={handleWheel}
      onDrop={handleDrop}
      onDragOver={handleDragOver}
    >
      {/* Ruler */}
      <div className="timeline-ruler-row">
        <div className="track-header-spacer" />
        <div
          className="timeline-ruler"
          ref={rulerRef}
          style={{ width: totalWidth }}
          onClick={handleRulerClick}
        >
          {rulerMarks}
          <div
            className="playback-cursor ruler-cursor"
            style={{ left: cursorX }}
          />
        </div>
      </div>

      {/* Tracks */}
      <div className="timeline-tracks-scroll">
        <div className="timeline-tracks" style={{ minWidth: totalWidth + 200 }}>
          {tracks.length === 0 && (
            <div className="timeline-empty">
              <p>No tracks yet</p>
              <p className="timeline-empty-hint">Click "Add Track" or drag & drop an audio file</p>
            </div>
          )}
          {tracks.map(track => (
            <Track
              key={track.id}
              track={track}
              zoom={zoom}
              totalWidth={totalWidth}
              isSelected={track.id === selectedTrackId}
              selectedRegionId={selectedRegionId}
              onSelect={() => onSelectTrack(track.id)}
              onSelectRegion={onSelectRegion}
              onUpdate={(updates) => onUpdateTrack(track.id, updates)}
              onRemove={() => onRemoveTrack(track.id)}
              onSplitRegion={(regionId, time) => onSplitRegion(track.id, regionId, time)}
              onRemoveRegion={(regionId) => onRemoveRegion(track.id, regionId)}
              onMoveRegion={onMoveRegion}
              cursorPosition={transport.position}
              onImportFile={onImportFile}
            />
          ))}
          {/* Playback cursor overlay */}
          <div
            className="playback-cursor track-cursor"
            ref={cursorRef}
            style={{ left: cursorX + 200 }} // offset by track header width
          />
        </div>
      </div>
    </div>
  );
}
