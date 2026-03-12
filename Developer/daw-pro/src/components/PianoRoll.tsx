import React, { useState, useCallback, useRef, useEffect, useMemo } from 'react';
import { useDAWStore } from '../store';
import { audioEngine } from '../engine/AudioEngine';
import { v4 as uuid } from 'uuid';
import { noteToName, NOTE_NAMES } from '../types';
import type { MidiNote, Clip } from '../types';
import './PianoRoll.css';

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */
const TOTAL_KEYS = 128;
const KEY_HEIGHT = 14;
const PIANO_WIDTH = 60;
const HEADER_HEIGHT = 24;
const VELOCITY_HEIGHT = 80;
const MIN_NOTE_DURATION = 0.0625; // 1/64
const DEFAULT_VELOCITY = 100;
const DEFAULT_DURATION = 0.25; // 1 beat quarter note

const GRID_OPTIONS: { label: string; value: number }[] = [
  { label: '1/4', value: 1 },
  { label: '1/8', value: 0.5 },
  { label: '1/16', value: 0.25 },
  { label: '1/32', value: 0.125 },
];

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */
function isBlackKey(note: number): boolean {
  const n = note % 12;
  return n === 1 || n === 3 || n === 6 || n === 8 || n === 10;
}

function snapValue(value: number, gridSize: number, enabled: boolean): number {
  if (!enabled) return value;
  return Math.round(value / gridSize) * gridSize;
}

function velocityToColor(velocity: number, baseColor: string): string {
  const ratio = velocity / 127;
  // Parse hex color
  const r = parseInt(baseColor.slice(1, 3), 16);
  const g = parseInt(baseColor.slice(3, 5), 16);
  const b = parseInt(baseColor.slice(5, 7), 16);
  // Darken for low velocity, brighten for high
  const factor = 0.3 + ratio * 0.7;
  const nr = Math.round(r * factor);
  const ng = Math.round(g * factor);
  const nb = Math.round(b * factor);
  return `rgb(${nr},${ng},${nb})`;
}

function velocityBarColor(velocity: number): string {
  const ratio = velocity / 127;
  if (ratio < 0.33) return '#3498db';
  if (ratio < 0.66) return '#f1c40f';
  return '#e74c3c';
}

/* ------------------------------------------------------------------ */
/*  Types                                                              */
/* ------------------------------------------------------------------ */
type InteractionMode = 'none' | 'drawing' | 'selecting' | 'moving' | 'resizing' | 'velocity';

interface DragState {
  mode: InteractionMode;
  noteId: string | null;
  startX: number;
  startY: number;
  startPitch: number;
  startTime: number;
  startDuration: number;
  currentX: number;
  currentY: number;
  offsetX: number;
  offsetY: number;
}

/* ------------------------------------------------------------------ */
/*  PianoRoll Component                                                */
/* ------------------------------------------------------------------ */
const PianoRoll: React.FC = () => {
  /* --- Store --- */
  const {
    selectedClipId,
    selectedTrackId,
    tracks,
    drawMode,
    snapToGrid,
    gridSize,
    horizontalZoom,
    addNote,
    removeNote,
    updateNote,
    moveNote,
    resizeNote,
    toggleDrawMode,
    toggleSnapToGrid,
    setGridSize,
  } = useDAWStore();

  /* --- Derived --- */
  const clip = useMemo<Clip | null>(() => {
    if (!selectedClipId) return null;
    for (const track of tracks) {
      for (const c of Object.values(track.clips)) {
        if (c.id === selectedClipId) return c;
      }
      const arr = track.arrangementClips.find((c) => c.id === selectedClipId);
      if (arr) return arr;
    }
    return null;
  }, [selectedClipId, tracks]);

  const notes = useMemo(() => clip?.notes ?? [], [clip]);
  const clipColor = clip?.color ?? '#3498db';
  const clipDuration = clip?.duration ?? 4; // bars worth of beats

  /* --- Local state --- */
  const [selectedNoteIds, setSelectedNoteIds] = useState<Set<string>>(new Set());
  const [scrollY, setScrollY] = useState(TOTAL_KEYS * KEY_HEIGHT / 2 - 200); // start near middle C
  const [scrollX, setScrollX] = useState(0);
  const [drag, setDrag] = useState<DragState | null>(null);

  /* --- Refs --- */
  const gridCanvasRef = useRef<HTMLCanvasElement>(null);
  const velocityCanvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const gridContainerRef = useRef<HTMLDivElement>(null);
  const velocityContainerRef = useRef<HTMLDivElement>(null);
  const animFrameRef = useRef<number>(0);

  /* --- Dimensions --- */
  const beatWidth = 60 * horizontalZoom;
  const totalBeats = Math.max(clipDuration * 4, 16); // show at least 16 beats
  const gridWidth = totalBeats * beatWidth;
  const totalHeight = TOTAL_KEYS * KEY_HEIGHT;

  /* ================================================================ */
  /*  Draw Grid Canvas                                                 */
  /* ================================================================ */
  const drawGrid = useCallback(() => {
    const canvas = gridCanvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const rect = canvas.parentElement?.getBoundingClientRect();
    if (!rect) return;
    const w = rect.width;
    const h = rect.height;
    canvas.width = w;
    canvas.height = h;

    ctx.clearRect(0, 0, w, h);

    const visibleStartBeat = scrollX / beatWidth;
    const visibleEndBeat = (scrollX + w) / beatWidth;
    const visibleStartNote = Math.floor(scrollY / KEY_HEIGHT);
    const visibleEndNote = Math.min(TOTAL_KEYS, Math.ceil((scrollY + h) / KEY_HEIGHT));

    /* --- Background rows --- */
    for (let note = visibleStartNote; note <= visibleEndNote; note++) {
      const pitch = TOTAL_KEYS - 1 - note;
      const y = note * KEY_HEIGHT - scrollY;
      if (y + KEY_HEIGHT < 0 || y > h) continue;
      const black = isBlackKey(pitch);
      ctx.fillStyle = black ? '#1a1a2e' : '#1e1e36';
      ctx.fillRect(0, y, w, KEY_HEIGHT);
      // Row separator
      ctx.strokeStyle = 'rgba(255,255,255,0.04)';
      ctx.lineWidth = 0.5;
      ctx.beginPath();
      ctx.moveTo(0, y + KEY_HEIGHT);
      ctx.lineTo(w, y + KEY_HEIGHT);
      ctx.stroke();
      // Octave boundary (C notes)
      if (pitch % 12 === 0) {
        ctx.strokeStyle = 'rgba(255,255,255,0.12)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(0, y + KEY_HEIGHT);
        ctx.lineTo(w, y + KEY_HEIGHT);
        ctx.stroke();
      }
    }

    /* --- Vertical grid lines --- */
    const startBeat = Math.floor(visibleStartBeat / gridSize) * gridSize;
    for (let beat = startBeat; beat <= visibleEndBeat; beat += gridSize) {
      const x = beat * beatWidth - scrollX;
      if (x < 0 || x > w) continue;
      const isBeatBoundary = Math.abs(beat - Math.round(beat)) < 0.001;
      const isBarBoundary = Math.abs(beat % 4) < 0.001;

      if (isBarBoundary) {
        ctx.strokeStyle = 'rgba(255,255,255,0.2)';
        ctx.lineWidth = 1;
      } else if (isBeatBoundary) {
        ctx.strokeStyle = 'rgba(255,255,255,0.1)';
        ctx.lineWidth = 0.5;
      } else {
        ctx.strokeStyle = 'rgba(255,255,255,0.04)';
        ctx.lineWidth = 0.5;
      }
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, h);
      ctx.stroke();
    }

    /* --- Notes --- */
    for (const note of notes) {
      const row = TOTAL_KEYS - 1 - note.pitch;
      const y = row * KEY_HEIGHT - scrollY;
      const x = note.startTime * beatWidth - scrollX;
      const noteW = note.duration * beatWidth;
      if (x + noteW < 0 || x > w || y + KEY_HEIGHT < 0 || y > h) continue;

      const isSelected = selectedNoteIds.has(note.id);
      const color = velocityToColor(note.velocity, clipColor);

      // Note body
      ctx.fillStyle = note.muted ? 'rgba(100,100,100,0.5)' : color;
      ctx.beginPath();
      const radius = 2;
      const nx = Math.max(0, x);
      const nw = Math.max(4, noteW);
      ctx.roundRect(nx, y + 1, nw, KEY_HEIGHT - 2, radius);
      ctx.fill();

      // Selected border
      if (isSelected) {
        ctx.strokeStyle = '#ffffff';
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.roundRect(nx, y + 1, nw, KEY_HEIGHT - 2, radius);
        ctx.stroke();
      }

      // Note name text (only if wide enough)
      if (nw > 24) {
        ctx.fillStyle = note.muted ? '#888' : '#fff';
        ctx.font = '9px Inter, system-ui, sans-serif';
        ctx.fillText(noteToName(note.pitch), nx + 3, y + KEY_HEIGHT - 4);
      }

      // Resize handle (right edge)
      ctx.fillStyle = 'rgba(255,255,255,0.3)';
      ctx.fillRect(nx + nw - 4, y + 1, 4, KEY_HEIGHT - 2);
    }
  }, [notes, scrollX, scrollY, beatWidth, gridSize, clipColor, selectedNoteIds, totalBeats]);

  /* ================================================================ */
  /*  Draw Velocity Canvas                                             */
  /* ================================================================ */
  const drawVelocity = useCallback(() => {
    const canvas = velocityCanvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const rect = canvas.parentElement?.getBoundingClientRect();
    if (!rect) return;
    const w = rect.width;
    const h = VELOCITY_HEIGHT;
    canvas.width = w;
    canvas.height = h;

    ctx.clearRect(0, 0, w, h);

    // Background
    ctx.fillStyle = '#0d0d1a';
    ctx.fillRect(0, 0, w, h);

    // Horizontal guides
    for (const level of [0.25, 0.5, 0.75]) {
      const y = h - level * h;
      ctx.strokeStyle = 'rgba(255,255,255,0.06)';
      ctx.lineWidth = 0.5;
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(w, y);
      ctx.stroke();
    }

    // Velocity bars
    for (const note of notes) {
      const x = note.startTime * beatWidth - scrollX;
      const barW = Math.max(4, note.duration * beatWidth - 2);
      if (x + barW < 0 || x > w) continue;

      const barH = (note.velocity / 127) * (h - 4);
      const isSelected = selectedNoteIds.has(note.id);
      const color = velocityBarColor(note.velocity);

      ctx.fillStyle = isSelected ? '#ffffff40' : 'transparent';
      ctx.fillRect(x, 0, barW, h);

      ctx.fillStyle = color;
      ctx.fillRect(x + 1, h - barH - 2, barW - 2, barH);

      // Velocity value on top of tall bars
      if (barH > 14) {
        ctx.fillStyle = '#fff';
        ctx.font = '8px Inter, system-ui, sans-serif';
        ctx.fillText(String(note.velocity), x + 3, h - barH - 4);
      }
    }
  }, [notes, scrollX, beatWidth, selectedNoteIds]);

  /* ================================================================ */
  /*  Render loop                                                      */
  /* ================================================================ */
  useEffect(() => {
    const render = () => {
      drawGrid();
      drawVelocity();
      animFrameRef.current = requestAnimationFrame(render);
    };
    animFrameRef.current = requestAnimationFrame(render);
    return () => cancelAnimationFrame(animFrameRef.current);
  }, [drawGrid, drawVelocity]);

  /* ================================================================ */
  /*  Mouse helpers                                                    */
  /* ================================================================ */
  const getGridPosition = useCallback(
    (clientX: number, clientY: number): { beat: number; pitch: number } => {
      const el = gridContainerRef.current;
      if (!el) return { beat: 0, pitch: 60 };
      const rect = el.getBoundingClientRect();
      const x = clientX - rect.left + scrollX;
      const y = clientY - rect.top + scrollY;
      const beat = x / beatWidth;
      const row = Math.floor(y / KEY_HEIGHT);
      const pitch = TOTAL_KEYS - 1 - row;
      return { beat: Math.max(0, beat), pitch: Math.max(0, Math.min(127, pitch)) };
    },
    [scrollX, scrollY, beatWidth]
  );

  const getNoteAt = useCallback(
    (beat: number, pitch: number): MidiNote | null => {
      for (const note of notes) {
        if (
          note.pitch === pitch &&
          beat >= note.startTime &&
          beat <= note.startTime + note.duration
        ) {
          return note;
        }
      }
      return null;
    },
    [notes]
  );

  const isOnResizeHandle = useCallback(
    (clientX: number, note: MidiNote): boolean => {
      const el = gridContainerRef.current;
      if (!el) return false;
      const rect = el.getBoundingClientRect();
      const x = clientX - rect.left + scrollX;
      const noteEnd = (note.startTime + note.duration) * beatWidth;
      return Math.abs(x - noteEnd) < 6;
    },
    [scrollX, beatWidth]
  );

  /* ================================================================ */
  /*  Grid mouse handlers                                              */
  /* ================================================================ */
  const handleGridMouseDown = useCallback(
    (e: React.MouseEvent) => {
      if (!clip || !selectedClipId) return;
      if (e.button === 2) return; // right click handled separately

      const { beat, pitch } = getGridPosition(e.clientX, e.clientY);
      const hitNote = getNoteAt(beat, pitch);

      if (drawMode) {
        if (hitNote) {
          // In draw mode, clicking existing note selects it
          setSelectedNoteIds(new Set([hitNote.id]));
        } else {
          // Create note
          const snappedBeat = snapValue(beat, gridSize, snapToGrid);
          const newNote: MidiNote = {
            id: uuid(),
            pitch,
            velocity: DEFAULT_VELOCITY,
            startTime: snappedBeat,
            duration: gridSize,
            probability: 1,
            muted: false,
          };
          addNote(selectedClipId, newNote);
          setSelectedNoteIds(new Set([newNote.id]));
          // Play the note for feedback
          if (selectedTrackId) {
            audioEngine.playNote(selectedTrackId, pitch, DEFAULT_VELOCITY / 127, 0.2);
          }
          setDrag({
            mode: 'drawing',
            noteId: newNote.id,
            startX: e.clientX,
            startY: e.clientY,
            startPitch: pitch,
            startTime: snappedBeat,
            startDuration: gridSize,
            currentX: e.clientX,
            currentY: e.clientY,
            offsetX: 0,
            offsetY: 0,
          });
        }
      } else {
        // Select mode
        if (hitNote) {
          if (isOnResizeHandle(e.clientX, hitNote)) {
            setSelectedNoteIds(new Set([hitNote.id]));
            setDrag({
              mode: 'resizing',
              noteId: hitNote.id,
              startX: e.clientX,
              startY: e.clientY,
              startPitch: hitNote.pitch,
              startTime: hitNote.startTime,
              startDuration: hitNote.duration,
              currentX: e.clientX,
              currentY: e.clientY,
              offsetX: 0,
              offsetY: 0,
            });
          } else {
            // Select / start move
            if (!e.shiftKey) {
              if (!selectedNoteIds.has(hitNote.id)) {
                setSelectedNoteIds(new Set([hitNote.id]));
              }
            } else {
              const next = new Set(selectedNoteIds);
              if (next.has(hitNote.id)) next.delete(hitNote.id);
              else next.add(hitNote.id);
              setSelectedNoteIds(next);
            }
            if (selectedTrackId) {
              audioEngine.playNote(selectedTrackId, hitNote.pitch, hitNote.velocity / 127, 0.15);
            }
            setDrag({
              mode: 'moving',
              noteId: hitNote.id,
              startX: e.clientX,
              startY: e.clientY,
              startPitch: hitNote.pitch,
              startTime: hitNote.startTime,
              startDuration: hitNote.duration,
              currentX: e.clientX,
              currentY: e.clientY,
              offsetX: beat - hitNote.startTime,
              offsetY: 0,
            });
          }
        } else {
          // Clicked empty space - deselect
          if (!e.shiftKey) {
            setSelectedNoteIds(new Set());
          }
        }
      }
    },
    [clip, selectedClipId, selectedTrackId, drawMode, snapToGrid, gridSize, notes, getGridPosition, getNoteAt, isOnResizeHandle, addNote, selectedNoteIds]
  );

  const handleGridMouseMove = useCallback(
    (e: React.MouseEvent) => {
      if (!drag || !selectedClipId) return;
      const { beat, pitch } = getGridPosition(e.clientX, e.clientY);

      if (drag.mode === 'drawing' && drag.noteId) {
        const snappedEnd = snapValue(beat, gridSize, snapToGrid);
        const newDuration = Math.max(MIN_NOTE_DURATION, snappedEnd - drag.startTime);
        resizeNote(selectedClipId, drag.noteId, newDuration);
      } else if (drag.mode === 'moving' && drag.noteId) {
        const deltaBeat = beat - drag.offsetX - drag.startTime;
        const deltaPitch = pitch - drag.startPitch;
        const snappedTime = snapValue(drag.startTime + deltaBeat, gridSize, snapToGrid);
        const newPitch = Math.max(0, Math.min(127, drag.startPitch + deltaPitch));
        // Move all selected notes
        selectedNoteIds.forEach((nid) => {
          const n = notes.find((nn) => nn.id === nid);
          if (!n) return;
          if (nid === drag.noteId) {
            moveNote(selectedClipId, nid, newPitch, Math.max(0, snappedTime));
          } else {
            const offsetBeat = n.startTime - drag.startTime;
            const offsetPitch = n.pitch - drag.startPitch;
            moveNote(
              selectedClipId,
              nid,
              Math.max(0, Math.min(127, newPitch + offsetPitch)),
              Math.max(0, snappedTime + offsetBeat)
            );
          }
        });
      } else if (drag.mode === 'resizing' && drag.noteId) {
        const snappedEnd = snapValue(beat, gridSize, snapToGrid);
        const newDuration = Math.max(MIN_NOTE_DURATION, snappedEnd - drag.startTime);
        resizeNote(selectedClipId, drag.noteId, newDuration);
      }
      setDrag((prev) => prev ? { ...prev, currentX: e.clientX, currentY: e.clientY } : null);
    },
    [drag, selectedClipId, getGridPosition, gridSize, snapToGrid, notes, selectedNoteIds, moveNote, resizeNote]
  );

  const handleGridMouseUp = useCallback(() => {
    setDrag(null);
  }, []);

  /* --- Context menu (delete) --- */
  const handleGridContextMenu = useCallback(
    (e: React.MouseEvent) => {
      e.preventDefault();
      if (!selectedClipId) return;
      const { beat, pitch } = getGridPosition(e.clientX, e.clientY);
      const hitNote = getNoteAt(beat, pitch);
      if (hitNote) {
        removeNote(selectedClipId, hitNote.id);
        setSelectedNoteIds((prev) => {
          const next = new Set(prev);
          next.delete(hitNote.id);
          return next;
        });
      }
    },
    [selectedClipId, getGridPosition, getNoteAt, removeNote]
  );

  /* ================================================================ */
  /*  Velocity mouse handlers                                          */
  /* ================================================================ */
  const handleVelocityMouseDown = useCallback(
    (e: React.MouseEvent) => {
      if (!selectedClipId) return;
      const el = velocityContainerRef.current;
      if (!el) return;
      const rect = el.getBoundingClientRect();
      const x = e.clientX - rect.left + scrollX;
      const y = e.clientY - rect.top;
      const beat = x / beatWidth;
      const hitNote = getNoteAt(beat, notes[0]?.pitch ?? 60);
      // Find note at this x position across all pitches
      let targetNote: MidiNote | null = null;
      for (const n of notes) {
        if (beat >= n.startTime && beat <= n.startTime + n.duration) {
          targetNote = n;
          break;
        }
      }
      if (!targetNote) return;
      const newVel = Math.max(1, Math.min(127, Math.round((1 - y / VELOCITY_HEIGHT) * 127)));
      updateNote(selectedClipId, targetNote.id, { velocity: newVel });
      setDrag({
        mode: 'velocity',
        noteId: targetNote.id,
        startX: e.clientX,
        startY: e.clientY,
        startPitch: 0,
        startTime: 0,
        startDuration: 0,
        currentX: e.clientX,
        currentY: e.clientY,
        offsetX: 0,
        offsetY: 0,
      });
    },
    [selectedClipId, notes, scrollX, beatWidth, getNoteAt, updateNote]
  );

  const handleVelocityMouseMove = useCallback(
    (e: React.MouseEvent) => {
      if (!drag || drag.mode !== 'velocity' || !selectedClipId) return;
      const el = velocityContainerRef.current;
      if (!el) return;
      const rect = el.getBoundingClientRect();
      const y = e.clientY - rect.top;
      const newVel = Math.max(1, Math.min(127, Math.round((1 - y / VELOCITY_HEIGHT) * 127)));
      if (drag.noteId) {
        updateNote(selectedClipId, drag.noteId, { velocity: newVel });
      }
    },
    [drag, selectedClipId, updateNote]
  );

  /* ================================================================ */
  /*  Keyboard handler                                                 */
  /* ================================================================ */
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (!selectedClipId) return;
      if (e.key === 'Delete' || e.key === 'Backspace') {
        selectedNoteIds.forEach((nid) => removeNote(selectedClipId, nid));
        setSelectedNoteIds(new Set());
      }
      if (e.key === 'a' && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        setSelectedNoteIds(new Set(notes.map((n) => n.id)));
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [selectedClipId, selectedNoteIds, notes, removeNote]);

  /* ================================================================ */
  /*  Scroll handler                                                   */
  /* ================================================================ */
  const handleScroll = useCallback((e: React.WheelEvent) => {
    e.preventDefault();
    if (e.shiftKey) {
      setScrollX((prev) => Math.max(0, prev + e.deltaY));
    } else {
      setScrollY((prev) => Math.max(0, Math.min(totalHeight - 200, prev + e.deltaY)));
    }
  }, [totalHeight]);

  /* ================================================================ */
  /*  Piano key click                                                  */
  /* ================================================================ */
  const handlePianoKeyClick = useCallback(
    (pitch: number) => {
      if (selectedTrackId) {
        audioEngine.playNote(selectedTrackId, pitch, 0.8, 0.3);
      }
    },
    [selectedTrackId]
  );

  /* ================================================================ */
  /*  Quantize                                                         */
  /* ================================================================ */
  const handleQuantize = useCallback(() => {
    if (!selectedClipId) return;
    const targetNotes = selectedNoteIds.size > 0
      ? notes.filter((n) => selectedNoteIds.has(n.id))
      : notes;
    targetNotes.forEach((n) => {
      const snappedTime = snapValue(n.startTime, gridSize, true);
      const snappedDuration = Math.max(MIN_NOTE_DURATION, snapValue(n.duration, gridSize, true));
      updateNote(selectedClipId, n.id, { startTime: snappedTime, duration: snappedDuration });
    });
  }, [selectedClipId, notes, selectedNoteIds, gridSize, updateNote]);

  /* ================================================================ */
  /*  Piano keyboard rendering                                         */
  /* ================================================================ */
  const pianoKeys = useMemo(() => {
    const keys: React.ReactNode[] = [];
    const visibleStart = Math.floor(scrollY / KEY_HEIGHT);
    const visibleEnd = Math.min(TOTAL_KEYS, visibleStart + Math.ceil(500 / KEY_HEIGHT) + 2);

    for (let row = visibleStart; row < visibleEnd; row++) {
      const pitch = TOTAL_KEYS - 1 - row;
      if (pitch < 0 || pitch > 127) continue;
      const black = isBlackKey(pitch);
      const isC = pitch % 12 === 0;
      const y = row * KEY_HEIGHT - scrollY;

      keys.push(
        <div
          key={pitch}
          className={`piano-key ${black ? 'black' : 'white'} ${isC ? 'c-note' : ''}`}
          style={{ top: y, height: KEY_HEIGHT }}
          onMouseDown={(e) => {
            e.preventDefault();
            handlePianoKeyClick(pitch);
          }}
        >
          {isC && <span className="piano-key-label">{noteToName(pitch)}</span>}
        </div>
      );
    }
    return keys;
  }, [scrollY, handlePianoKeyClick]);

  /* ================================================================ */
  /*  No clip selected                                                 */
  /* ================================================================ */
  if (!clip || clip.type !== 'midi') {
    return (
      <div className="piano-roll-empty">
        <div className="piano-roll-empty-text">
          Select a MIDI clip to edit
        </div>
      </div>
    );
  }

  /* ================================================================ */
  /*  Render                                                           */
  /* ================================================================ */
  return (
    <div className="piano-roll" ref={containerRef}>
      {/* --- Toolbar --- */}
      <div className="piano-roll-toolbar">
        <div className="pr-toolbar-left">
          <span className="pr-clip-name">{clip.name}</span>
          <div className="pr-toolbar-sep" />
          <button
            className={`pr-tool-btn ${drawMode ? 'active' : ''}`}
            onClick={toggleDrawMode}
            title="Draw mode (D)"
          >
            <svg width="14" height="14" viewBox="0 0 14 14">
              <path d="M11.5 1.5l1 1-8 8-2 1 1-2z" fill="currentColor" />
            </svg>
          </button>
          <button
            className={`pr-tool-btn ${snapToGrid ? 'active' : ''}`}
            onClick={toggleSnapToGrid}
            title="Snap to grid"
          >
            <svg width="14" height="14" viewBox="0 0 14 14">
              <path d="M1 1h4v4H1zM9 1h4v4H9zM1 9h4v4H1zM9 9h4v4H9z" fill="currentColor" opacity="0.6" />
            </svg>
          </button>
          <div className="pr-toolbar-sep" />
          <span className="pr-toolbar-label">Grid:</span>
          <select
            className="pr-grid-select"
            value={gridSize}
            onChange={(e) => setGridSize(Number(e.target.value))}
          >
            {GRID_OPTIONS.map((opt) => (
              <option key={opt.value} value={opt.value}>{opt.label}</option>
            ))}
          </select>
        </div>
        <div className="pr-toolbar-right">
          <button className="pr-tool-btn" onClick={handleQuantize} title="Quantize">
            Q
          </button>
          <span className="pr-note-count">{notes.length} notes</span>
        </div>
      </div>

      {/* --- Main area --- */}
      <div className="piano-roll-body">
        {/* Piano keyboard */}
        <div className="piano-keyboard" onWheel={handleScroll}>
          <div className="piano-keys-inner" style={{ height: totalHeight }}>
            {pianoKeys}
          </div>
        </div>

        {/* Grid + Velocity column */}
        <div className="piano-roll-grid-column">
          {/* Beat header */}
          <div className="pr-beat-header">
            {Array.from({ length: Math.ceil(totalBeats) }, (_, i) => {
              const x = i * beatWidth - scrollX;
              if (x < -beatWidth || x > 2000) return null;
              return (
                <span
                  key={i}
                  className="pr-beat-number"
                  style={{ left: x }}
                >
                  {i % 4 === 0 ? `${Math.floor(i / 4) + 1}` : ''}
                </span>
              );
            })}
          </div>

          {/* Note grid */}
          <div
            className="piano-roll-grid"
            ref={gridContainerRef}
            onWheel={handleScroll}
            onMouseDown={handleGridMouseDown}
            onMouseMove={handleGridMouseMove}
            onMouseUp={handleGridMouseUp}
            onMouseLeave={handleGridMouseUp}
            onContextMenu={handleGridContextMenu}
            style={{ cursor: drawMode ? 'crosshair' : drag?.mode === 'resizing' ? 'ew-resize' : 'default' }}
          >
            <canvas ref={gridCanvasRef} className="pr-grid-canvas" />
          </div>

          {/* Velocity editor */}
          <div className="pr-velocity-header">
            <span className="pr-velocity-label">Velocity</span>
          </div>
          <div
            className="piano-roll-velocity"
            ref={velocityContainerRef}
            onMouseDown={handleVelocityMouseDown}
            onMouseMove={handleVelocityMouseMove}
            onMouseUp={handleGridMouseUp}
            onMouseLeave={handleGridMouseUp}
          >
            <canvas ref={velocityCanvasRef} className="pr-velocity-canvas" />
          </div>
        </div>
      </div>
    </div>
  );
};

export default React.memo(PianoRoll);
