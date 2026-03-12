import React, { useState, useRef, useEffect, useCallback, useMemo } from 'react';
import { v4 as uuid } from 'uuid';
import { useDAWStore } from '../store';
import type { Track, Clip, MidiNote } from '../types';
import './ArrangementView.css';

const TRACK_HEADER_WIDTH = 180;
const BEAT_WIDTH_BASE = 40; // px per beat at zoom 1.0
const DEFAULT_TRACK_HEIGHT = 80;
const RULER_HEIGHT = 28;
const MIN_BAR_LABEL_SPACING = 60;

/** Quantize a beat position to the nearest grid line */
function snapBeat(beat: number, gridSize: number, enabled: boolean): number {
  if (!enabled) return beat;
  return Math.round(beat / gridSize) * gridSize;
}

/* ===== Waveform placeholder drawn on a canvas ===== */
const AudioWaveform: React.FC<{ width: number; height: number; color: string; clip: Clip }> = React.memo(
  ({ width, height, color }) => {
    const canvasRef = useRef<HTMLCanvasElement>(null);

    useEffect(() => {
      const canvas = canvasRef.current;
      if (!canvas || width <= 0 || height <= 0) return;
      canvas.width = width * window.devicePixelRatio;
      canvas.height = height * window.devicePixelRatio;
      const ctx = canvas.getContext('2d');
      if (!ctx) return;
      ctx.scale(window.devicePixelRatio, window.devicePixelRatio);
      ctx.clearRect(0, 0, width, height);

      // Draw a procedural waveform placeholder
      const mid = height / 2;
      ctx.beginPath();
      ctx.strokeStyle = color;
      ctx.lineWidth = 1;
      ctx.globalAlpha = 0.7;

      const segments = Math.max(2, Math.floor(width / 2));
      for (let i = 0; i <= segments; i++) {
        const x = (i / segments) * width;
        // Multi-frequency sine composite for realistic look
        const t = (i / segments) * Math.PI * 16;
        const amp =
          Math.sin(t) * 0.4 +
          Math.sin(t * 2.3 + 1) * 0.25 +
          Math.sin(t * 5.1 + 2) * 0.15 +
          Math.sin(t * 0.7) * 0.2;
        const y = mid + amp * (mid * 0.8);
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();

      // Mirror waveform for filled look
      ctx.beginPath();
      ctx.fillStyle = color;
      ctx.globalAlpha = 0.15;
      ctx.moveTo(0, mid);
      for (let i = 0; i <= segments; i++) {
        const x = (i / segments) * width;
        const t = (i / segments) * Math.PI * 16;
        const amp =
          Math.abs(
            Math.sin(t) * 0.4 +
            Math.sin(t * 2.3 + 1) * 0.25 +
            Math.sin(t * 5.1 + 2) * 0.15 +
            Math.sin(t * 0.7) * 0.2
          );
        ctx.lineTo(x, mid - amp * (mid * 0.8));
      }
      for (let i = segments; i >= 0; i--) {
        const x = (i / segments) * width;
        const t = (i / segments) * Math.PI * 16;
        const amp =
          Math.abs(
            Math.sin(t) * 0.4 +
            Math.sin(t * 2.3 + 1) * 0.25 +
            Math.sin(t * 5.1 + 2) * 0.15 +
            Math.sin(t * 0.7) * 0.2
          );
        ctx.lineTo(x, mid + amp * (mid * 0.8));
      }
      ctx.closePath();
      ctx.fill();
    }, [width, height, color]);

    return (
      <canvas
        ref={canvasRef}
        className="arrangement-audio-waveform"
        style={{ width, height }}
      />
    );
  }
);

/* ===== MIDI note blocks rendered inside a clip ===== */
const MidiNoteBlocks: React.FC<{ notes: MidiNote[]; clipWidth: number; clipHeight: number; duration: number; color: string }> = React.memo(
  ({ notes, clipWidth, clipHeight, duration, color }) => {
    if (!notes || notes.length === 0) return null;

    // Determine pitch range
    const pitches = notes.map((n) => n.pitch);
    const minPitch = Math.min(...pitches);
    const maxPitch = Math.max(...pitches);
    const pitchRange = Math.max(maxPitch - minPitch + 1, 12);
    const noteHeight = Math.max(2, Math.min(6, (clipHeight - 8) / pitchRange));

    return (
      <div className="arrangement-midi-notes">
        {notes.map((note) => {
          const left = (note.startTime / duration) * clipWidth;
          const width = Math.max(2, (note.duration / duration) * clipWidth);
          const pitchNorm = (note.pitch - minPitch) / pitchRange;
          const top = (1 - pitchNorm) * (clipHeight - noteHeight - 8) + 4;
          return (
            <div
              key={note.id}
              className="arrangement-midi-note-block"
              style={{
                left,
                top,
                width,
                height: noteHeight,
                backgroundColor: color,
                opacity: note.muted ? 0.3 : 0.85,
              }}
            />
          );
        })}
      </div>
    );
  }
);

/* ===== Arrangement Clip ===== */
const ArrangementClip: React.FC<{
  clip: Clip;
  trackId: string;
  beatWidth: number;
  trackHeight: number;
  isSelected: boolean;
  snapToGrid: boolean;
  gridSize: number;
  onSelect: (clipId: string) => void;
  onMove: (trackId: string, clipId: string, newStart: number) => void;
  onDelete: (trackId: string, clipId: string) => void;
}> = React.memo(({ clip, trackId, beatWidth, trackHeight, isSelected, snapToGrid, gridSize, onSelect, onMove, onDelete }) => {
  const [isDragging, setIsDragging] = useState(false);
  const dragRef = useRef({ startX: 0, startBeat: 0 });

  const clipWidth = clip.duration * beatWidth;
  const clipLeft = clip.startTime * beatWidth;
  const contentHeight = trackHeight - 4;

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    if (e.button !== 0) return;
    e.stopPropagation();
    onSelect(clip.id);
    setIsDragging(true);
    dragRef.current = { startX: e.clientX, startBeat: clip.startTime };

    const handleMouseMove = (ev: MouseEvent) => {
      const dx = ev.clientX - dragRef.current.startX;
      const deltaBeat = dx / beatWidth;
      let newStart = Math.max(0, dragRef.current.startBeat + deltaBeat);
      newStart = snapBeat(newStart, gridSize, snapToGrid);
      onMove(trackId, clip.id, newStart);
    };

    const handleMouseUp = () => {
      setIsDragging(false);
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('mouseup', handleMouseUp);
    };

    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', handleMouseUp);
  }, [clip.id, clip.startTime, trackId, beatWidth, gridSize, snapToGrid, onSelect, onMove]);

  const handleContextMenu = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    e.stopPropagation();
    onDelete(trackId, clip.id);
  }, [trackId, clip.id, onDelete]);

  return (
    <div
      className={`arrangement-clip ${clip.type} ${isSelected ? 'selected' : ''} ${isDragging ? 'dragging' : ''}`}
      style={{
        left: clipLeft,
        width: Math.max(clipWidth, 4),
        height: contentHeight,
        backgroundColor: clip.color + 'cc',
        borderLeftColor: clip.color,
      }}
      onMouseDown={handleMouseDown}
      onContextMenu={handleContextMenu}
    >
      <div className="arrangement-clip-header">
        <span className="arrangement-clip-name">{clip.name}</span>
      </div>
      <div className="arrangement-clip-content">
        {clip.type === 'audio' && (
          <AudioWaveform
            width={Math.max(clipWidth - 4, 0)}
            height={Math.max(contentHeight - 18, 0)}
            color={clip.color}
            clip={clip}
          />
        )}
        {clip.type === 'midi' && clip.notes && (
          <MidiNoteBlocks
            notes={clip.notes}
            clipWidth={Math.max(clipWidth - 4, 0)}
            clipHeight={Math.max(contentHeight - 18, 0)}
            duration={clip.duration}
            color={clip.color}
          />
        )}
      </div>
      {clip.loopEnabled && (
        <div className="arrangement-clip-loop-marker" />
      )}
    </div>
  );
});

/* ===== Track Header ===== */
const ArrangementTrackHeader: React.FC<{
  track: Track;
  height: number;
  isSelected: boolean;
  onSelect: (id: string) => void;
  onToggleMute: (id: string) => void;
  onToggleSolo: (id: string) => void;
  onToggleArm: (id: string) => void;
}> = React.memo(({ track, height, isSelected, onSelect, onToggleMute, onToggleSolo, onToggleArm }) => (
  <div
    className={`arrangement-track-header ${isSelected ? 'selected' : ''}`}
    style={{ height }}
    onClick={() => onSelect(track.id)}
  >
    <div className="arrangement-track-header-color" style={{ backgroundColor: track.color }} />
    <div className="arrangement-track-header-info">
      <span className="arrangement-track-header-name">{track.name}</span>
      <div className="arrangement-track-header-buttons">
        <button
          className={`arr-track-btn arr-arm-btn ${track.armed ? 'active' : ''}`}
          onClick={(e) => { e.stopPropagation(); onToggleArm(track.id); }}
        >
          <svg width="8" height="8" viewBox="0 0 10 10">
            <circle cx="5" cy="5" r="4" fill="none" stroke="currentColor" strokeWidth="1.5" />
          </svg>
        </button>
        <button
          className={`arr-track-btn arr-solo-btn ${track.solo ? 'active' : ''}`}
          onClick={(e) => { e.stopPropagation(); onToggleSolo(track.id); }}
        >
          S
        </button>
        <button
          className={`arr-track-btn arr-mute-btn ${track.mute ? 'active' : ''}`}
          onClick={(e) => { e.stopPropagation(); onToggleMute(track.id); }}
        >
          M
        </button>
      </div>
    </div>
  </div>
));

/* ===== Timeline Ruler ===== */
const TimelineRuler: React.FC<{
  scrollX: number;
  beatWidth: number;
  width: number;
  timeSignature: [number, number];
  loopEnabled: boolean;
  loopStart: number;
  loopEnd: number;
  onSetBeat: (beat: number) => void;
}> = React.memo(({ scrollX, beatWidth, width, timeSignature, loopEnabled, loopStart, loopEnd, onSetBeat }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const beatsPerBar = timeSignature[0];

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const dpr = window.devicePixelRatio || 1;
    canvas.width = width * dpr;
    canvas.height = RULER_HEIGHT * dpr;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    ctx.scale(dpr, dpr);
    ctx.clearRect(0, 0, width, RULER_HEIGHT);

    // Background
    ctx.fillStyle = '#0d1426';
    ctx.fillRect(0, 0, width, RULER_HEIGHT);

    // Determine visible range in beats
    const startBeat = scrollX / beatWidth;
    const endBeat = startBeat + width / beatWidth;

    // Determine label density
    const barWidth = beatWidth * beatsPerBar;
    let labelInterval = 1;
    if (barWidth < MIN_BAR_LABEL_SPACING) {
      labelInterval = Math.ceil(MIN_BAR_LABEL_SPACING / barWidth);
    }

    const startBar = Math.floor(startBeat / beatsPerBar);
    const endBar = Math.ceil(endBeat / beatsPerBar) + 1;

    // Loop region
    if (loopEnabled) {
      const lx1 = (loopStart - startBeat) * beatWidth;
      const lx2 = (loopEnd - startBeat) * beatWidth;
      ctx.fillStyle = 'rgba(52, 152, 219, 0.15)';
      ctx.fillRect(lx1, 0, lx2 - lx1, RULER_HEIGHT);
      // Loop edges
      ctx.strokeStyle = '#3498db';
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      ctx.moveTo(lx1, 0);
      ctx.lineTo(lx1, RULER_HEIGHT);
      ctx.moveTo(lx2, 0);
      ctx.lineTo(lx2, RULER_HEIGHT);
      ctx.stroke();
    }

    // Draw bar/beat markers
    for (let bar = startBar; bar <= endBar; bar++) {
      const barBeat = bar * beatsPerBar;
      const x = (barBeat - startBeat) * beatWidth;

      if (x < -1 || x > width + 1) continue;

      // Bar line
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.25)';
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(x, RULER_HEIGHT - 10);
      ctx.lineTo(x, RULER_HEIGHT);
      ctx.stroke();

      // Bar number label
      if (bar >= 0 && bar % labelInterval === 0) {
        ctx.fillStyle = '#aaa';
        ctx.font = '10px Inter, system-ui, sans-serif';
        ctx.textAlign = 'left';
        ctx.fillText(`${bar + 1}`, x + 4, 14);
      }

      // Beat subdivisions
      if (beatWidth > 15) {
        for (let b = 1; b < beatsPerBar; b++) {
          const bx = x + b * beatWidth;
          if (bx < 0 || bx > width) continue;
          ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
          ctx.lineWidth = 0.5;
          ctx.beginPath();
          ctx.moveTo(bx, RULER_HEIGHT - 6);
          ctx.lineTo(bx, RULER_HEIGHT);
          ctx.stroke();
        }
      }
    }

    // Bottom line
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.15)';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(0, RULER_HEIGHT - 0.5);
    ctx.lineTo(width, RULER_HEIGHT - 0.5);
    ctx.stroke();
  }, [scrollX, beatWidth, width, beatsPerBar, loopEnabled, loopStart, loopEnd]);

  const handleClick = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    const rect = e.currentTarget.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const beat = (scrollX + x) / beatWidth;
    onSetBeat(Math.max(0, beat));
  }, [scrollX, beatWidth, onSetBeat]);

  return (
    <canvas
      ref={canvasRef}
      className="arrangement-ruler"
      style={{ width, height: RULER_HEIGHT }}
      onClick={handleClick}
    />
  );
});

/* ===== Grid Lines on the timeline area ===== */
const GridLines: React.FC<{
  scrollX: number;
  beatWidth: number;
  width: number;
  height: number;
  timeSignature: [number, number];
}> = React.memo(({ scrollX, beatWidth, width, height, timeSignature }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const beatsPerBar = timeSignature[0];

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || width <= 0 || height <= 0) return;
    const dpr = window.devicePixelRatio || 1;
    canvas.width = width * dpr;
    canvas.height = height * dpr;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    ctx.scale(dpr, dpr);
    ctx.clearRect(0, 0, width, height);

    const startBeat = scrollX / beatWidth;
    const endBeat = startBeat + width / beatWidth;
    const startBar = Math.floor(startBeat / beatsPerBar);
    const endBar = Math.ceil(endBeat / beatsPerBar) + 1;

    for (let bar = startBar; bar <= endBar; bar++) {
      const barBeat = bar * beatsPerBar;
      const x = (barBeat - startBeat) * beatWidth;

      if (x >= -1 && x <= width + 1) {
        // Bar line (stronger)
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.12)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, height);
        ctx.stroke();
      }

      // Beat lines (lighter)
      if (beatWidth > 12) {
        for (let b = 1; b < beatsPerBar; b++) {
          const bx = x + b * beatWidth;
          if (bx < 0 || bx > width) continue;
          ctx.strokeStyle = 'rgba(255, 255, 255, 0.04)';
          ctx.lineWidth = 0.5;
          ctx.beginPath();
          ctx.moveTo(bx, 0);
          ctx.lineTo(bx, height);
          ctx.stroke();
        }
      }

      // Sub-beat lines at high zoom
      if (beatWidth > 40) {
        for (let b = 0; b < beatsPerBar; b++) {
          for (let sub = 1; sub < 4; sub++) {
            const sbx = x + (b + sub / 4) * beatWidth;
            if (sbx < 0 || sbx > width) continue;
            ctx.strokeStyle = 'rgba(255, 255, 255, 0.02)';
            ctx.lineWidth = 0.5;
            ctx.beginPath();
            ctx.moveTo(sbx, 0);
            ctx.lineTo(sbx, height);
            ctx.stroke();
          }
        }
      }
    }
  }, [scrollX, beatWidth, width, height, beatsPerBar]);

  return (
    <canvas
      ref={canvasRef}
      className="arrangement-grid-lines"
      style={{ width, height, position: 'absolute', top: 0, left: 0, pointerEvents: 'none' }}
    />
  );
});

/* ===== Main ArrangementView component ===== */
const ArrangementView: React.FC = () => {
  const tracks = useDAWStore((s) => s.tracks);
  const isPlaying = useDAWStore((s) => s.isPlaying);
  const currentBeat = useDAWStore((s) => s.currentBeat);
  const horizontalZoom = useDAWStore((s) => s.horizontalZoom);
  const verticalZoom = useDAWStore((s) => s.verticalZoom);
  const scrollX = useDAWStore((s) => s.scrollX);
  const scrollY = useDAWStore((s) => s.scrollY);
  const selectedTrackId = useDAWStore((s) => s.selectedTrackId);
  const selectedClipId = useDAWStore((s) => s.selectedClipId);
  const drawMode = useDAWStore((s) => s.drawMode);
  const snapToGrid = useDAWStore((s) => s.snapToGrid);
  const gridSize = useDAWStore((s) => s.gridSize);
  const loopEnabled = useDAWStore((s) => s.loopEnabled);
  const loopStart = useDAWStore((s) => s.loopStart);
  const loopEnd = useDAWStore((s) => s.loopEnd);
  const timeSignature = useDAWStore((s) => s.timeSignature);

  const selectTrack = useDAWStore((s) => s.selectTrack);
  const selectClip = useDAWStore((s) => s.selectClip);
  const toggleTrackMute = useDAWStore((s) => s.toggleTrackMute);
  const toggleTrackSolo = useDAWStore((s) => s.toggleTrackSolo);
  const toggleTrackArm = useDAWStore((s) => s.toggleTrackArm);
  const setCurrentBeat = useDAWStore((s) => s.setCurrentBeat);
  const setScrollXAction = useDAWStore((s) => s.setScrollX);
  const setScrollYAction = useDAWStore((s) => s.setScrollY);
  const setHorizontalZoom = useDAWStore((s) => s.setHorizontalZoom);
  const moveArrangementClip = useDAWStore((s) => s.moveArrangementClip);
  const deleteArrangementClip = useDAWStore((s) => s.deleteArrangementClip);
  const addArrangementClip = useDAWStore((s) => s.addArrangementClip);

  const containerRef = useRef<HTMLDivElement>(null);
  const timelineRef = useRef<HTMLDivElement>(null);
  const [timelineWidth, setTimelineWidth] = useState(800);
  const [timelineContentHeight, setTimelineContentHeight] = useState(400);

  const beatWidth = BEAT_WIDTH_BASE * horizontalZoom;

  // Measure timeline width on mount / resize
  useEffect(() => {
    const measure = () => {
      if (timelineRef.current) {
        setTimelineWidth(timelineRef.current.clientWidth);
      }
    };
    measure();
    const observer = new ResizeObserver(measure);
    if (timelineRef.current) observer.observe(timelineRef.current);
    return () => observer.disconnect();
  }, []);

  // Compute total height of all tracks
  const trackHeights = useMemo(() => {
    return tracks.map((t) => (t.collapsed ? 24 : (t.height || DEFAULT_TRACK_HEIGHT) * verticalZoom));
  }, [tracks, verticalZoom]);

  const totalContentHeight = useMemo(() => {
    return trackHeights.reduce((sum, h) => sum + h, 0);
  }, [trackHeights]);

  useEffect(() => {
    setTimelineContentHeight(totalContentHeight);
  }, [totalContentHeight]);

  // Scroll handler
  const handleScroll = useCallback((e: React.UIEvent<HTMLDivElement>) => {
    const target = e.currentTarget;
    setScrollXAction(target.scrollLeft);
    setScrollYAction(target.scrollTop);
  }, [setScrollXAction, setScrollYAction]);

  // Zoom on wheel + Ctrl/Meta
  const handleWheel = useCallback((e: React.WheelEvent) => {
    if (e.ctrlKey || e.metaKey) {
      e.preventDefault();
      const delta = -e.deltaY * 0.002;
      setHorizontalZoom(horizontalZoom + delta);
    }
  }, [horizontalZoom, setHorizontalZoom]);

  // Draw mode: create clip on empty lane click
  const handleTimelineClick = useCallback((e: React.MouseEvent) => {
    if (!drawMode) return;
    const rect = (e.currentTarget as HTMLElement).getBoundingClientRect();
    const clickX = e.clientX - rect.left + scrollX;
    const clickY = e.clientY - rect.top + scrollY;

    let cumY = 0;
    let targetTrack: Track | null = null;
    for (let i = 0; i < tracks.length; i++) {
      const h = trackHeights[i];
      if (clickY >= cumY && clickY < cumY + h) {
        targetTrack = tracks[i];
        break;
      }
      cumY += h;
    }
    if (!targetTrack) return;

    const beat = clickX / beatWidth;
    const snappedBeat = snapBeat(beat, gridSize * timeSignature[0], snapToGrid);
    const beatsPerBar = timeSignature[0];
    const duration = beatsPerBar; // default 1 bar

    // Check if there is already a clip at this position
    const overlap = targetTrack.arrangementClips.some(
      (c) => snappedBeat < c.startTime + c.duration && snappedBeat + duration > c.startTime
    );
    if (overlap) return;

    const newClip: Clip = {
      id: uuid(),
      trackId: targetTrack.id,
      name: targetTrack.type === 'midi' ? 'MIDI Clip' : 'Audio Clip',
      color: targetTrack.color,
      type: targetTrack.type === 'audio' ? 'audio' : 'midi',
      startTime: snappedBeat,
      duration,
      loopEnabled: true,
      loopStart: 0,
      loopEnd: duration,
      launchMode: 'trigger',
      notes: targetTrack.type !== 'audio' ? [] : undefined,
      isPlaying: false,
      isRecording: false,
      automation: [],
    };

    addArrangementClip(targetTrack.id, newClip);
    selectClip(newClip.id);
  }, [drawMode, scrollX, scrollY, tracks, trackHeights, beatWidth, gridSize, timeSignature, snapToGrid, addArrangementClip, selectClip]);

  // Playhead position in px relative to scroll
  const playheadX = currentBeat * beatWidth - scrollX;

  // Loop region overlay
  const loopLeftPx = loopStart * beatWidth - scrollX;
  const loopWidthPx = (loopEnd - loopStart) * beatWidth;

  // Total timeline length (enough to show content + extra)
  const maxBeat = useMemo(() => {
    let max = 32; // minimum 32 beats (8 bars)
    tracks.forEach((t) => {
      t.arrangementClips.forEach((c) => {
        const end = c.startTime + c.duration;
        if (end > max) max = end;
      });
    });
    return max + 16; // extra padding
  }, [tracks]);

  const totalTimelineWidth = maxBeat * beatWidth;

  return (
    <div className="arrangement-view" ref={containerRef}>
      <div className="arrangement-layout">
        {/* Track Headers Column (fixed left) */}
        <div className="arrangement-headers-column">
          {/* Ruler spacer */}
          <div className="arrangement-ruler-spacer" style={{ height: RULER_HEIGHT }}>
            <span className="arrangement-ruler-label">Arrangement</span>
          </div>
          {/* Track headers */}
          <div className="arrangement-headers-scroll" style={{ top: -scrollY }}>
            {tracks.map((track, idx) => (
              <ArrangementTrackHeader
                key={track.id}
                track={track}
                height={trackHeights[idx]}
                isSelected={selectedTrackId === track.id}
                onSelect={selectTrack}
                onToggleMute={toggleTrackMute}
                onToggleSolo={toggleTrackSolo}
                onToggleArm={toggleTrackArm}
              />
            ))}
          </div>
        </div>

        {/* Timeline Area */}
        <div className="arrangement-timeline-area" ref={timelineRef} onWheel={handleWheel}>
          {/* Ruler (fixed at top) */}
          <div className="arrangement-ruler-container" style={{ height: RULER_HEIGHT }}>
            <TimelineRuler
              scrollX={scrollX}
              beatWidth={beatWidth}
              width={timelineWidth}
              timeSignature={timeSignature}
              loopEnabled={loopEnabled}
              loopStart={loopStart}
              loopEnd={loopEnd}
              onSetBeat={setCurrentBeat}
            />
          </div>

          {/* Scrollable Timeline Content */}
          <div
            className="arrangement-timeline-scroll"
            onScroll={handleScroll}
            onClick={handleTimelineClick}
          >
            <div
              className="arrangement-timeline-content"
              style={{
                width: totalTimelineWidth,
                height: totalContentHeight,
              }}
            >
              {/* Grid Lines */}
              <GridLines
                scrollX={0}
                beatWidth={beatWidth}
                width={totalTimelineWidth}
                height={totalContentHeight}
                timeSignature={timeSignature}
              />

              {/* Loop Region Overlay */}
              {loopEnabled && (
                <div
                  className="arrangement-loop-region"
                  style={{
                    left: loopStart * beatWidth,
                    width: (loopEnd - loopStart) * beatWidth,
                    height: totalContentHeight,
                  }}
                />
              )}

              {/* Track Lanes */}
              {tracks.map((track, trackIndex) => {
                const trackTop = trackHeights.slice(0, trackIndex).reduce((s, h) => s + h, 0);
                const trackH = trackHeights[trackIndex];
                return (
                  <div
                    key={track.id}
                    className={`arrangement-track-lane ${trackIndex % 2 === 0 ? 'even' : 'odd'} ${track.mute ? 'muted' : ''}`}
                    style={{
                      top: trackTop,
                      height: trackH,
                      width: totalTimelineWidth,
                    }}
                  >
                    {/* Clips */}
                    {track.arrangementClips.map((clip) => (
                      <ArrangementClip
                        key={clip.id}
                        clip={clip}
                        trackId={track.id}
                        beatWidth={beatWidth}
                        trackHeight={trackH}
                        isSelected={selectedClipId === clip.id}
                        snapToGrid={snapToGrid}
                        gridSize={gridSize * timeSignature[0]}
                        onSelect={selectClip}
                        onMove={moveArrangementClip}
                        onDelete={deleteArrangementClip}
                      />
                    ))}
                  </div>
                );
              })}

              {/* Playhead */}
              <div
                className={`arrangement-playhead ${isPlaying ? 'playing' : 'stopped'}`}
                style={{ left: currentBeat * beatWidth }}
              >
                <div className="arrangement-playhead-handle" />
                <div className="arrangement-playhead-line" style={{ height: totalContentHeight }} />
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default ArrangementView;
