import React, { useState, useCallback, useRef, useEffect, useMemo } from 'react';
import { useDAWStore } from '../store';
import type { Track } from '../types';
import { volumeToDb } from '../types';
import './MixerView.css';

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */
const FADER_HEIGHT = 140;
const METER_HEIGHT = 140;
const METER_WIDTH = 6;
const KNOB_SIZE = 28;
const SEND_KNOB_SIZE = 20;
const PEAK_HOLD_TIME = 1500; // ms

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */
function formatDb(volume: number): string {
  if (volume <= 0) return '-inf';
  const db = volumeToDb(volume);
  if (!isFinite(db)) return '-inf';
  return db.toFixed(1);
}

function meterToSegments(level: number): { green: number; yellow: number; orange: number; red: number } {
  // level 0-1 mapped to segments
  return {
    green: Math.min(level, 0.6) / 0.6,
    yellow: level > 0.6 ? Math.min((level - 0.6) / 0.15, 1) : 0,
    orange: level > 0.75 ? Math.min((level - 0.75) / 0.15, 1) : 0,
    red: level > 0.9 ? Math.min((level - 0.9) / 0.1, 1) : 0,
  };
}

/* ------------------------------------------------------------------ */
/*  Rotary Knob Component                                              */
/* ------------------------------------------------------------------ */
const RotaryKnob: React.FC<{
  value: number;
  min: number;
  max: number;
  size?: number;
  label?: string;
  onChange: (value: number) => void;
  formatValue?: (v: number) => string;
  centerZero?: boolean;
}> = React.memo(({ value, min, max, size = KNOB_SIZE, label, onChange, formatValue, centerZero }) => {
  const knobRef = useRef<HTMLDivElement>(null);
  const dragRef = useRef<{ startY: number; startValue: number } | null>(null);

  const normalizedValue = (value - min) / (max - min);
  const startAngle = -135;
  const endAngle = 135;
  const angle = startAngle + normalizedValue * (endAngle - startAngle);

  const handleMouseDown = useCallback(
    (e: React.MouseEvent) => {
      e.preventDefault();
      dragRef.current = { startY: e.clientY, startValue: value };

      const handleMove = (ev: MouseEvent) => {
        if (!dragRef.current) return;
        const dy = dragRef.current.startY - ev.clientY;
        const range = max - min;
        const sensitivity = ev.shiftKey ? 0.001 : 0.005;
        const newValue = Math.max(min, Math.min(max, dragRef.current.startValue + dy * range * sensitivity));
        onChange(newValue);
      };

      const handleUp = () => {
        dragRef.current = null;
        window.removeEventListener('mousemove', handleMove);
        window.removeEventListener('mouseup', handleUp);
      };

      window.addEventListener('mousemove', handleMove);
      window.addEventListener('mouseup', handleUp);
    },
    [value, min, max, onChange]
  );

  const handleDoubleClick = useCallback(() => {
    onChange(centerZero ? 0 : (min + max) / 2);
  }, [centerZero, min, max, onChange]);

  const r = size / 2 - 3;
  const cx = size / 2;
  const cy = size / 2;

  // Arc path for the track
  const arcStart = -135 * (Math.PI / 180) - Math.PI / 2;
  const arcEnd = 135 * (Math.PI / 180) - Math.PI / 2;
  const arcValue = (startAngle + normalizedValue * (endAngle - startAngle)) * (Math.PI / 180) - Math.PI / 2;

  const describeArc = (startA: number, endA: number, radius: number) => {
    const x1 = cx + radius * Math.cos(startA);
    const y1 = cy + radius * Math.sin(startA);
    const x2 = cx + radius * Math.cos(endA);
    const y2 = cy + radius * Math.sin(endA);
    const largeArc = endA - startA > Math.PI ? 1 : 0;
    return `M ${x1} ${y1} A ${radius} ${radius} 0 ${largeArc} 1 ${x2} ${y2}`;
  };

  // Indicator line end point
  const indicatorAngle = angle * (Math.PI / 180) - Math.PI / 2;
  const ix = cx + (r - 2) * Math.cos(indicatorAngle);
  const iy = cy + (r - 2) * Math.sin(indicatorAngle);
  const ix2 = cx + (r - 7) * Math.cos(indicatorAngle);
  const iy2 = cy + (r - 7) * Math.sin(indicatorAngle);

  const displayValue = formatValue ? formatValue(value) : value.toFixed(1);

  return (
    <div className="rotary-knob-wrapper" style={{ width: size }}>
      <div
        className="rotary-knob"
        ref={knobRef}
        style={{ width: size, height: size }}
        onMouseDown={handleMouseDown}
        onDoubleClick={handleDoubleClick}
        title={displayValue}
      >
        <svg width={size} height={size}>
          {/* Track background */}
          <path
            d={describeArc(arcStart, arcEnd, r)}
            fill="none"
            stroke="rgba(255,255,255,0.1)"
            strokeWidth="2"
            strokeLinecap="round"
          />
          {/* Value arc */}
          {centerZero ? (
            <>
              {normalizedValue >= 0.5 && (
                <path
                  d={describeArc(-Math.PI / 2, arcValue, r)}
                  fill="none"
                  stroke="#5dade2"
                  strokeWidth="2"
                  strokeLinecap="round"
                />
              )}
              {normalizedValue < 0.5 && (
                <path
                  d={describeArc(arcValue, -Math.PI / 2, r)}
                  fill="none"
                  stroke="#5dade2"
                  strokeWidth="2"
                  strokeLinecap="round"
                />
              )}
            </>
          ) : (
            <path
              d={describeArc(arcStart, arcValue, r)}
              fill="none"
              stroke="#5dade2"
              strokeWidth="2"
              strokeLinecap="round"
            />
          )}
          {/* Knob body */}
          <circle cx={cx} cy={cy} r={r - 4} fill="#1a1a2e" stroke="rgba(255,255,255,0.15)" strokeWidth="1" />
          {/* Indicator line */}
          <line x1={ix2} y1={iy2} x2={ix} y2={iy} stroke="#fff" strokeWidth="1.5" strokeLinecap="round" />
        </svg>
      </div>
      {label && <span className="knob-label">{label}</span>}
    </div>
  );
});

/* ------------------------------------------------------------------ */
/*  Level Meter Component                                              */
/* ------------------------------------------------------------------ */
const LevelMeter: React.FC<{
  left: number;
  right: number;
  height?: number;
}> = React.memo(({ left, right, height = METER_HEIGHT }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const peakLeftRef = useRef(0);
  const peakRightRef = useRef(0);
  const peakLeftTimeRef = useRef(0);
  const peakRightTimeRef = useRef(0);
  const animRef = useRef<number>(0);

  useEffect(() => {
    const now = Date.now();
    if (left > peakLeftRef.current) {
      peakLeftRef.current = left;
      peakLeftTimeRef.current = now;
    } else if (now - peakLeftTimeRef.current > PEAK_HOLD_TIME) {
      peakLeftRef.current = Math.max(left, peakLeftRef.current * 0.95);
    }
    if (right > peakRightRef.current) {
      peakRightRef.current = right;
      peakRightTimeRef.current = now;
    } else if (now - peakRightTimeRef.current > PEAK_HOLD_TIME) {
      peakRightRef.current = Math.max(right, peakRightRef.current * 0.95);
    }
  }, [left, right]);

  useEffect(() => {
    const draw = () => {
      const canvas = canvasRef.current;
      if (!canvas) return;
      const ctx = canvas.getContext('2d');
      if (!ctx) return;

      const w = 16;
      const h = height;
      canvas.width = w;
      canvas.height = h;

      ctx.clearRect(0, 0, w, h);

      // Background
      ctx.fillStyle = '#0a0a14';
      ctx.fillRect(0, 0, w, h);

      const drawBar = (x: number, barW: number, level: number, peak: number) => {
        const barH = level * h;
        const segmentH = 2;
        const gap = 1;
        const totalSegments = Math.floor(h / (segmentH + gap));
        const activeSegments = Math.floor(level * totalSegments);

        for (let i = 0; i < activeSegments; i++) {
          const y = h - (i + 1) * (segmentH + gap);
          const ratio = i / totalSegments;
          if (ratio < 0.6) {
            ctx.fillStyle = '#2ecc71';
          } else if (ratio < 0.75) {
            ctx.fillStyle = '#f1c40f';
          } else if (ratio < 0.9) {
            ctx.fillStyle = '#e67e22';
          } else {
            ctx.fillStyle = '#e74c3c';
          }
          ctx.fillRect(x, y, barW, segmentH);
        }

        // Peak hold indicator
        if (peak > 0.01) {
          const peakY = h - peak * h;
          const peakRatio = peak;
          if (peakRatio < 0.6) ctx.fillStyle = '#2ecc71';
          else if (peakRatio < 0.75) ctx.fillStyle = '#f1c40f';
          else if (peakRatio < 0.9) ctx.fillStyle = '#e67e22';
          else ctx.fillStyle = '#e74c3c';
          ctx.fillRect(x, peakY, barW, 2);
        }
      };

      // Left channel
      drawBar(1, METER_WIDTH, left, peakLeftRef.current);
      // Right channel
      drawBar(9, METER_WIDTH, right, peakRightRef.current);

      animRef.current = requestAnimationFrame(draw);
    };

    animRef.current = requestAnimationFrame(draw);
    return () => cancelAnimationFrame(animRef.current);
  }, [left, right, height]);

  return (
    <div className="level-meter" style={{ height }}>
      <canvas ref={canvasRef} width={16} height={height} />
    </div>
  );
});

/* ------------------------------------------------------------------ */
/*  Volume Fader Component                                             */
/* ------------------------------------------------------------------ */
const VolumeFader: React.FC<{
  value: number;
  onChange: (value: number) => void;
  height?: number;
}> = React.memo(({ value, onChange, height = FADER_HEIGHT }) => {
  const trackRef = useRef<HTMLDivElement>(null);
  const dragging = useRef(false);

  const thumbPosition = (1 - value) * (height - 20);

  const handleMouseDown = useCallback(
    (e: React.MouseEvent) => {
      e.preventDefault();
      dragging.current = true;

      const handleMove = (ev: MouseEvent) => {
        if (!dragging.current || !trackRef.current) return;
        const rect = trackRef.current.getBoundingClientRect();
        const y = ev.clientY - rect.top;
        const normalized = Math.max(0, Math.min(1, 1 - y / (height - 20)));
        onChange(normalized);
      };

      const handleUp = () => {
        dragging.current = false;
        window.removeEventListener('mousemove', handleMove);
        window.removeEventListener('mouseup', handleUp);
      };

      window.addEventListener('mousemove', handleMove);
      window.addEventListener('mouseup', handleUp);

      // Immediately set value
      const rect = trackRef.current?.getBoundingClientRect();
      if (rect) {
        const y = e.clientY - rect.top;
        const normalized = Math.max(0, Math.min(1, 1 - y / (height - 20)));
        onChange(normalized);
      }
    },
    [onChange, height]
  );

  const handleDoubleClick = useCallback(() => {
    onChange(0.8); // default
  }, [onChange]);

  return (
    <div className="volume-fader" style={{ height }}>
      <div className="fader-track" ref={trackRef} onMouseDown={handleMouseDown}>
        {/* dB scale marks */}
        <div className="fader-scale">
          {['+6', '0', '-6', '-12', '-24', '-inf'].map((label, i) => (
            <span key={label} className="fader-scale-mark" style={{ top: `${(i / 5) * 100}%` }}>
              {label}
            </span>
          ))}
        </div>
        {/* Fill */}
        <div className="fader-fill" style={{ height: `${value * 100}%` }} />
        {/* Thumb */}
        <div
          className="fader-thumb"
          style={{ top: thumbPosition }}
          onDoubleClick={handleDoubleClick}
        />
      </div>
      <span className="fader-value">{formatDb(value)}</span>
    </div>
  );
});

/* ------------------------------------------------------------------ */
/*  Channel Strip Component                                            */
/* ------------------------------------------------------------------ */
const ChannelStrip: React.FC<{
  track: Track;
  index: number;
  isMaster?: boolean;
  isReturn?: boolean;
  isSelected: boolean;
  returnTracks: Track[];
}> = React.memo(({ track, index, isMaster, isReturn, isSelected, returnTracks }) => {
  const {
    selectTrack,
    setTrackVolume,
    setTrackPan,
    toggleTrackMute,
    toggleTrackSolo,
    toggleTrackArm,
    renameTrack,
  } = useDAWStore();

  const [isEditing, setIsEditing] = useState(false);
  const [editName, setEditName] = useState(track.name);
  const inputRef = useRef<HTMLInputElement>(null);

  /* --- Track name editing --- */
  const handleDoubleClick = useCallback(() => {
    setEditName(track.name);
    setIsEditing(true);
  }, [track.name]);

  useEffect(() => {
    if (isEditing && inputRef.current) {
      inputRef.current.focus();
      inputRef.current.select();
    }
  }, [isEditing]);

  const handleNameSubmit = useCallback(() => {
    if (editName.trim() && editName !== track.name) {
      renameTrack(track.id, editName.trim());
    }
    setIsEditing(false);
  }, [editName, track.id, track.name, renameTrack]);

  const handleNameKeyDown = useCallback(
    (e: React.KeyboardEvent) => {
      if (e.key === 'Enter') handleNameSubmit();
      if (e.key === 'Escape') setIsEditing(false);
    },
    [handleNameSubmit]
  );

  /* --- Handlers --- */
  const handleSelect = useCallback(() => selectTrack(track.id), [track.id, selectTrack]);
  const handleVolumeChange = useCallback((v: number) => setTrackVolume(track.id, v), [track.id, setTrackVolume]);
  const handlePanChange = useCallback((v: number) => setTrackPan(track.id, v), [track.id, setTrackPan]);
  const handleMute = useCallback(() => toggleTrackMute(track.id), [track.id, toggleTrackMute]);
  const handleSolo = useCallback(() => toggleTrackSolo(track.id), [track.id, toggleTrackSolo]);
  const handleArm = useCallback(() => toggleTrackArm(track.id), [track.id, toggleTrackArm]);

  const formatPan = useCallback((v: number) => {
    if (Math.abs(v) < 0.01) return 'C';
    return v < 0 ? `${Math.round(Math.abs(v) * 50)}L` : `${Math.round(v * 50)}R`;
  }, []);

  const trackLabel = isReturn
    ? String.fromCharCode(65 + index) // A, B, C...
    : isMaster
      ? 'M'
      : String(index + 1);

  return (
    <div
      className={`channel-strip ${isSelected ? 'selected' : ''} ${isMaster ? 'master' : ''} ${isReturn ? 'return' : ''}`}
      onClick={handleSelect}
    >
      {/* Color indicator */}
      <div className="strip-color" style={{ backgroundColor: track.color }} />

      {/* Track name */}
      <div className="strip-name" onDoubleClick={handleDoubleClick}>
        {isEditing ? (
          <input
            ref={inputRef}
            className="strip-name-input"
            value={editName}
            onChange={(e) => setEditName(e.target.value)}
            onBlur={handleNameSubmit}
            onKeyDown={handleNameKeyDown}
          />
        ) : (
          <span className="strip-name-text">{track.name}</span>
        )}
      </div>

      {/* I/O label */}
      <div className="strip-io">
        <span className="io-label">{track.type === 'midi' ? 'MIDI' : track.type === 'audio' ? 'Audio' : track.type}</span>
      </div>

      {/* Device count */}
      <div className="strip-devices">
        <span className="device-count">
          {track.devices.length > 0 ? `${track.devices.length} device${track.devices.length > 1 ? 's' : ''}` : 'No devices'}
        </span>
      </div>

      {/* Send knobs (not for master or return) */}
      {!isMaster && !isReturn && returnTracks.length > 0 && (
        <div className="strip-sends">
          {returnTracks.map((rt, i) => {
            const send = track.sends?.find((s) => s.returnTrackId === rt.id);
            const sendAmount = send?.amount ?? 0;
            return (
              <div key={rt.id} className="send-row">
                <span className="send-label">{String.fromCharCode(65 + i)}</span>
                <RotaryKnob
                  value={sendAmount}
                  min={0}
                  max={1}
                  size={SEND_KNOB_SIZE}
                  onChange={() => {/* Send amount change - would need store action */}}
                  formatValue={(v) => `${Math.round(v * 100)}%`}
                />
              </div>
            );
          })}
        </div>
      )}

      {/* Pan knob */}
      {!isMaster && (
        <div className="strip-pan">
          <RotaryKnob
            value={track.pan}
            min={-1}
            max={1}
            size={KNOB_SIZE}
            label="Pan"
            onChange={handlePanChange}
            formatValue={formatPan}
            centerZero
          />
        </div>
      )}

      {/* Meter + Fader area */}
      <div className="strip-meter-fader">
        <LevelMeter left={track.meterLeft} right={track.meterRight} />
        <VolumeFader value={track.volume} onChange={handleVolumeChange} />
      </div>

      {/* S / M / Arm buttons */}
      <div className="strip-buttons">
        {!isMaster && (
          <button
            className={`strip-btn solo-btn ${track.solo ? 'active' : ''}`}
            onClick={(e) => { e.stopPropagation(); handleSolo(); }}
            title="Solo"
          >
            S
          </button>
        )}
        {!isMaster && (
          <button
            className={`strip-btn mute-btn ${track.mute ? 'active' : ''}`}
            onClick={(e) => { e.stopPropagation(); handleMute(); }}
            title="Mute"
          >
            M
          </button>
        )}
        {!isMaster && !isReturn && (
          <button
            className={`strip-btn arm-btn ${track.armed ? 'active' : ''}`}
            onClick={(e) => { e.stopPropagation(); handleArm(); }}
            title="Record Arm"
          >
            <span className="arm-dot" />
          </button>
        )}
      </div>

      {/* Track number */}
      <div className="strip-number">{trackLabel}</div>
    </div>
  );
});

/* ------------------------------------------------------------------ */
/*  MixerView Component                                                */
/* ------------------------------------------------------------------ */
const MixerView: React.FC = () => {
  const {
    tracks,
    masterTrack,
    returnTracks,
    selectedTrackId,
  } = useDAWStore();

  const scrollRef = useRef<HTMLDivElement>(null);

  return (
    <div className="mixer-view">
      {/* Mixer header */}
      <div className="mixer-header">
        <span className="mixer-title">Mixer</span>
        <span className="mixer-track-count">{tracks.length} tracks</span>
      </div>

      {/* Channel strips container */}
      <div className="mixer-strips-container" ref={scrollRef}>
        <div className="mixer-strips">
          {/* Regular tracks */}
          {tracks.map((track, i) => (
            <ChannelStrip
              key={track.id}
              track={track}
              index={i}
              isSelected={track.id === selectedTrackId}
              returnTracks={returnTracks}
            />
          ))}

          {/* Separator */}
          {returnTracks.length > 0 && <div className="mixer-separator" />}

          {/* Return tracks */}
          {returnTracks.map((track, i) => (
            <ChannelStrip
              key={track.id}
              track={track}
              index={i}
              isReturn
              isSelected={track.id === selectedTrackId}
              returnTracks={[]}
            />
          ))}

          {/* Master separator */}
          <div className="mixer-separator master-sep" />

          {/* Master track */}
          <ChannelStrip
            track={masterTrack}
            index={0}
            isMaster
            isSelected={masterTrack.id === selectedTrackId}
            returnTracks={[]}
          />
        </div>
      </div>
    </div>
  );
};

export default React.memo(MixerView);
