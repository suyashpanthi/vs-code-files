import React, { useState, useRef, useEffect, useCallback } from 'react';
import { useDAWStore } from '../store';
import { audioEngine } from '../engine/AudioEngine';
import './Transport.css';

const GRID_SIZES = [
  { label: '1/32', value: 0.125 },
  { label: '1/16', value: 0.25 },
  { label: '1/8', value: 0.5 },
  { label: '1/4', value: 1 },
  { label: '1/2', value: 2 },
  { label: '1 Bar', value: 4 },
];

function formatPosition(beat: number, timeSignature: [number, number]): string {
  const beatsPerBar = timeSignature[0];
  const bar = Math.floor(beat / beatsPerBar) + 1;
  const beatInBar = Math.floor(beat % beatsPerBar) + 1;
  const sub = Math.floor((beat % 1) * 4) + 1;
  return `${String(bar).padStart(3, ' ')}.${beatInBar}.${sub}`;
}

export default function Transport() {
  const {
    bpm, setBPM,
    timeSignature,
    isPlaying, isRecording,
    loopEnabled, currentBeat,
    metronomeEnabled, followPlayhead,
    drawMode, snapToGrid, gridSize,
    activeView,
    togglePlay, stop, record,
    toggleLoop, toggleMetronome, toggleFollowPlayhead,
    setCurrentBeat, toggleDrawMode, toggleSnapToGrid,
    setGridSize, toggleView,
  } = useDAWStore();

  const [editingBPM, setEditingBPM] = useState(false);
  const [bpmInput, setBpmInput] = useState(String(bpm));
  const [showGridDropdown, setShowGridDropdown] = useState(false);
  const [cpuLoad] = useState(4);
  const bpmInputRef = useRef<HTMLInputElement>(null);
  const tapTimesRef = useRef<number[]>([]);
  const animFrameRef = useRef<number>(0);
  const gridDropdownRef = useRef<HTMLDivElement>(null);

  // Position tracking via requestAnimationFrame
  useEffect(() => {
    let running = true;
    const tick = () => {
      if (!running) return;
      if (audioEngine.isPlaying) {
        const pos = audioEngine.getPosition();
        setCurrentBeat(pos);
      }
      animFrameRef.current = requestAnimationFrame(tick);
    };
    animFrameRef.current = requestAnimationFrame(tick);
    return () => {
      running = false;
      cancelAnimationFrame(animFrameRef.current);
    };
  }, [setCurrentBeat]);

  // Close grid dropdown on outside click
  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (gridDropdownRef.current && !gridDropdownRef.current.contains(e.target as Node)) {
        setShowGridDropdown(false);
      }
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, []);

  // Focus BPM input when editing
  useEffect(() => {
    if (editingBPM && bpmInputRef.current) {
      bpmInputRef.current.focus();
      bpmInputRef.current.select();
    }
  }, [editingBPM]);

  const handleTapTempo = useCallback(() => {
    const now = performance.now();
    const taps = tapTimesRef.current;
    // Reset taps if more than 2 seconds since last tap
    if (taps.length > 0 && now - taps[taps.length - 1] > 2000) {
      tapTimesRef.current = [];
    }
    tapTimesRef.current.push(now);
    const current = tapTimesRef.current;
    if (current.length >= 2) {
      const intervals: number[] = [];
      for (let i = 1; i < current.length; i++) {
        intervals.push(current[i] - current[i - 1]);
      }
      // Keep only last 8 taps
      if (current.length > 8) {
        tapTimesRef.current = current.slice(-8);
      }
      const avgInterval = intervals.reduce((a, b) => a + b, 0) / intervals.length;
      const detectedBPM = Math.round(60000 / avgInterval);
      if (detectedBPM >= 20 && detectedBPM <= 999) {
        setBPM(detectedBPM);
        setBpmInput(String(detectedBPM));
      }
    }
  }, [setBPM]);

  const handleBPMClick = useCallback(() => {
    setEditingBPM(true);
    setBpmInput(String(bpm));
  }, [bpm]);

  const handleBPMSubmit = useCallback(() => {
    const val = parseFloat(bpmInput);
    if (!isNaN(val) && val >= 20 && val <= 999) {
      setBPM(val);
    } else {
      setBpmInput(String(bpm));
    }
    setEditingBPM(false);
  }, [bpmInput, bpm, setBPM]);

  const handleBPMKeyDown = useCallback((e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      handleBPMSubmit();
    } else if (e.key === 'Escape') {
      setBpmInput(String(bpm));
      setEditingBPM(false);
    }
  }, [handleBPMSubmit, bpm]);

  const handleBPMWheel = useCallback((e: React.WheelEvent) => {
    e.preventDefault();
    const delta = e.deltaY < 0 ? 1 : -1;
    const newBPM = Math.max(20, Math.min(999, bpm + delta));
    setBPM(newBPM);
    setBpmInput(String(newBPM));
  }, [bpm, setBPM]);

  const handleBackToStart = useCallback(() => {
    setCurrentBeat(0);
    audioEngine.setPosition(0);
  }, [setCurrentBeat]);

  const handleTogglePlay = useCallback(async () => {
    try {
      await audioEngine.init();
    } catch {
      // already initialized
    }
    if (isPlaying) {
      audioEngine.pause();
    } else {
      audioEngine.setTempo(bpm);
      audioEngine.play();
    }
    togglePlay();
  }, [isPlaying, bpm, togglePlay]);

  const handleStop = useCallback(() => {
    audioEngine.stop();
    stop();
  }, [stop]);

  const handleRecord = useCallback(async () => {
    try {
      await audioEngine.init();
    } catch {
      // already initialized
    }
    record();
  }, [record]);

  const currentGridLabel = GRID_SIZES.find(g => g.value === gridSize)?.label || '1/16';

  return (
    <div className="transport-bar">
      {/* Left Section */}
      <div className="transport-section transport-left">
        <button
          className="transport-btn tap-tempo-btn"
          onClick={handleTapTempo}
          title="Tap Tempo"
        >
          TAP
        </button>

        <div
          className="bpm-display"
          onWheel={handleBPMWheel}
          title="Click to edit, scroll to adjust"
        >
          {editingBPM ? (
            <input
              ref={bpmInputRef}
              className="bpm-input"
              type="text"
              value={bpmInput}
              onChange={(e) => setBpmInput(e.target.value)}
              onBlur={handleBPMSubmit}
              onKeyDown={handleBPMKeyDown}
            />
          ) : (
            <span className="bpm-value" onClick={handleBPMClick}>
              {bpm.toFixed(0)}
            </span>
          )}
          <span className="bpm-label">BPM</span>
        </div>

        <div className="time-sig-display" title="Time Signature">
          {timeSignature[0]}/{timeSignature[1]}
        </div>

        <button
          className={`transport-btn metronome-btn ${metronomeEnabled ? 'active' : ''}`}
          onClick={toggleMetronome}
          title="Metronome"
        >
          M
        </button>
      </div>

      {/* Center Section */}
      <div className="transport-section transport-center">
        <button
          className="transport-btn back-btn"
          onClick={handleBackToStart}
          title="Back to Start"
        >
          |&#9666;
        </button>

        <button
          className={`transport-btn play-btn ${isPlaying ? 'active' : ''}`}
          onClick={isPlaying ? handleStop : handleTogglePlay}
          title={isPlaying ? 'Stop' : 'Play'}
        >
          {isPlaying ? '\u25A0' : '\u25B6'}
        </button>

        <button
          className={`transport-btn record-btn ${isRecording ? 'active' : ''}`}
          onClick={handleRecord}
          title="Record"
        >
          &#9679;
        </button>

        <button
          className={`transport-btn loop-btn ${loopEnabled ? 'active' : ''}`}
          onClick={toggleLoop}
          title="Loop"
        >
          &#8635;
        </button>

        <div className="position-display" title="Position: Bar.Beat.Sub">
          {formatPosition(currentBeat, timeSignature)}
        </div>
      </div>

      {/* Right Section */}
      <div className="transport-section transport-right">
        <button
          className={`transport-btn follow-btn ${followPlayhead ? 'active' : ''}`}
          onClick={toggleFollowPlayhead}
          title="Follow Playhead"
        >
          &#8594;|
        </button>

        <button
          className={`transport-btn draw-btn ${drawMode ? 'active' : ''}`}
          onClick={toggleDrawMode}
          title="Draw Mode"
        >
          &#9998;
        </button>

        <button
          className={`transport-btn snap-btn ${snapToGrid ? 'active' : ''}`}
          onClick={toggleSnapToGrid}
          title="Snap to Grid"
        >
          &#9641;
        </button>

        <div className="grid-selector" ref={gridDropdownRef}>
          <button
            className="transport-btn grid-btn"
            onClick={() => setShowGridDropdown(!showGridDropdown)}
            title="Grid Size"
          >
            {currentGridLabel}
          </button>
          {showGridDropdown && (
            <div className="grid-dropdown">
              {GRID_SIZES.map((gs) => (
                <button
                  key={gs.value}
                  className={`grid-option ${gridSize === gs.value ? 'selected' : ''}`}
                  onClick={() => {
                    setGridSize(gs.value);
                    setShowGridDropdown(false);
                  }}
                >
                  {gs.label}
                </button>
              ))}
            </div>
          )}
        </div>

        <div className="cpu-indicator" title="CPU Usage">
          <span className="cpu-label">CPU</span>
          <span className="cpu-value">{cpuLoad}%</span>
        </div>

        <button
          className="transport-btn view-btn"
          onClick={toggleView}
          title={activeView === 'session' ? 'Switch to Arrangement' : 'Switch to Session'}
        >
          {activeView === 'session' ? 'SES' : 'ARR'}
        </button>
      </div>
    </div>
  );
}
