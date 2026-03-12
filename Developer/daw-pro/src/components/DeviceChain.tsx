import React, { useState, useCallback, useRef, useEffect } from 'react';
import { v4 as uuid } from 'uuid';
import { useDAWStore } from '../store';
import { audioEngine, getEffectParameters, getSynthParameters, SynthType, EffectType } from '../engine/AudioEngine';
import type { Device, DeviceParameter } from '../types';
import './DeviceChain.css';

// SVG Rotary Knob Component
interface KnobProps {
  parameter: DeviceParameter;
  onChange: (value: number) => void;
  size?: number;
}

function Knob({ parameter, onChange, size = 40 }: KnobProps) {
  const { name, value, min, max, defaultValue, step, unit } = parameter;
  const knobRef = useRef<SVGSVGElement>(null);
  const dragRef = useRef<{ startY: number; startValue: number; fine: boolean } | null>(null);

  const normalizedValue = (value - min) / (max - min);
  const startAngle = -135;
  const endAngle = 135;
  const angle = startAngle + normalizedValue * (endAngle - startAngle);

  const radius = size / 2 - 4;
  const cx = size / 2;
  const cy = size / 2;

  // Arc path for the track
  const polarToCartesian = (cx: number, cy: number, r: number, angleDeg: number) => {
    const rad = (angleDeg - 90) * (Math.PI / 180);
    return { x: cx + r * Math.cos(rad), y: cy + r * Math.sin(rad) };
  };

  const arcPath = (startA: number, endA: number, r: number) => {
    const start = polarToCartesian(cx, cy, r, endA);
    const end = polarToCartesian(cx, cy, r, startA);
    const largeArc = endA - startA > 180 ? 1 : 0;
    return `M ${start.x} ${start.y} A ${r} ${r} 0 ${largeArc} 0 ${end.x} ${end.y}`;
  };

  const trackPath = arcPath(startAngle, endAngle, radius);
  const valuePath = normalizedValue > 0.001 ? arcPath(startAngle, angle, radius) : '';

  // Indicator line
  const indicatorEnd = polarToCartesian(cx, cy, radius - 4, angle);
  const indicatorStart = polarToCartesian(cx, cy, radius * 0.35, angle);

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    dragRef.current = {
      startY: e.clientY,
      startValue: value,
      fine: e.shiftKey,
    };

    const handleMouseMove = (e: MouseEvent) => {
      if (!dragRef.current) return;
      const fine = e.shiftKey;
      const sensitivity = fine ? 0.001 : 0.005;
      const deltaY = dragRef.current.startY - e.clientY;
      const range = max - min;
      const delta = deltaY * sensitivity * range;
      let newValue = dragRef.current.startValue + delta;
      if (step) {
        newValue = Math.round(newValue / step) * step;
      }
      newValue = Math.max(min, Math.min(max, newValue));
      onChange(newValue);
    };

    const handleMouseUp = () => {
      dragRef.current = null;
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };

    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
  }, [value, min, max, step, onChange]);

  const handleDoubleClick = useCallback(() => {
    onChange(defaultValue);
  }, [defaultValue, onChange]);

  const formatValue = (v: number): string => {
    if (Math.abs(v) >= 1000) return `${(v / 1000).toFixed(1)}k`;
    if (Number.isInteger(v) || (step && step >= 1)) return v.toFixed(0);
    if (step && step >= 0.1) return v.toFixed(1);
    return v.toFixed(2);
  };

  return (
    <div className="knob-container">
      <svg
        ref={knobRef}
        className="knob-svg"
        width={size}
        height={size}
        onMouseDown={handleMouseDown}
        onDoubleClick={handleDoubleClick}
        style={{ cursor: 'ns-resize' }}
      >
        {/* Track background */}
        <path
          d={trackPath}
          fill="none"
          stroke="#2d333b"
          strokeWidth={3}
          strokeLinecap="round"
        />
        {/* Value arc */}
        {valuePath && (
          <path
            d={valuePath}
            fill="none"
            stroke="#4493f8"
            strokeWidth={3}
            strokeLinecap="round"
          />
        )}
        {/* Knob body */}
        <circle
          cx={cx}
          cy={cy}
          r={radius * 0.55}
          fill="#1c2128"
          stroke="#3d444d"
          strokeWidth={1.5}
        />
        {/* Indicator */}
        <line
          x1={indicatorStart.x}
          y1={indicatorStart.y}
          x2={indicatorEnd.x}
          y2={indicatorEnd.y}
          stroke="#e6edf3"
          strokeWidth={2}
          strokeLinecap="round"
        />
      </svg>
      <span className="knob-label">{name}</span>
      <span className="knob-value">{formatValue(value)}{unit ? ` ${unit}` : ''}</span>
    </div>
  );
}

// Add Device Dropdown
interface AddDeviceDropdownProps {
  onAdd: (device: Device) => void;
  onClose: () => void;
}

function AddDeviceDropdown({ onAdd, onClose }: AddDeviceDropdownProps) {
  const dropdownRef = useRef<HTMLDivElement>(null);
  const [search, setSearch] = useState('');

  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (dropdownRef.current && !dropdownRef.current.contains(e.target as Node)) {
        onClose();
      }
    };
    document.addEventListener('mousedown', handler);
    return () => document.removeEventListener('mousedown', handler);
  }, [onClose]);

  const instruments: { name: string; type: SynthType }[] = [
    { name: 'Poly Synth', type: 'poly' },
    { name: 'FM Synth', type: 'fm' },
    { name: 'AM Synth', type: 'am' },
    { name: 'Mono Synth', type: 'mono' },
    { name: 'Membrane', type: 'membrane' },
    { name: 'Metal', type: 'metal' },
    { name: 'Pluck', type: 'pluck' },
    { name: 'Noise', type: 'noise' },
    { name: 'Duo', type: 'duo' },
    { name: 'Wavetable', type: 'wavetable' },
  ];

  const effects: { name: string; type: EffectType }[] = [
    { name: 'Reverb', type: 'reverb' },
    { name: 'Delay', type: 'delay' },
    { name: 'Chorus', type: 'chorus' },
    { name: 'Phaser', type: 'phaser' },
    { name: 'Distortion', type: 'distortion' },
    { name: 'Compressor', type: 'compressor' },
    { name: 'EQ3', type: 'eq3' },
    { name: 'Filter', type: 'filter' },
    { name: 'Tremolo', type: 'tremolo' },
    { name: 'Vibrato', type: 'vibrato' },
    { name: 'BitCrusher', type: 'bitcrusher' },
    { name: 'Chebyshev', type: 'chebyshev' },
    { name: 'FreeVerb', type: 'freeverb' },
    { name: 'Ping Pong Delay', type: 'pingpong' },
    { name: 'AutoWah', type: 'autowah' },
    { name: 'Pitch Shift', type: 'pitchshift' },
  ];

  const query = search.toLowerCase();
  const filteredInstruments = instruments.filter(i => i.name.toLowerCase().includes(query));
  const filteredEffects = effects.filter(e => e.name.toLowerCase().includes(query));

  const handleAddInstrument = (item: { name: string; type: SynthType }) => {
    const params = getSynthParameters(item.type);
    const device: Device = {
      id: uuid(),
      name: item.name,
      type: 'instrument',
      subType: item.type as any,
      enabled: true,
      collapsed: false,
      parameters: params,
    };
    onAdd(device);
    onClose();
  };

  const handleAddEffect = (item: { name: string; type: EffectType }) => {
    const params = getEffectParameters(item.type);
    const device: Device = {
      id: uuid(),
      name: item.name,
      type: 'audioEffect',
      subType: item.type as any,
      enabled: true,
      collapsed: false,
      parameters: params,
    };
    onAdd(device);
    onClose();
  };

  return (
    <div className="add-device-dropdown" ref={dropdownRef}>
      <input
        className="add-device-search"
        type="text"
        placeholder="Search devices..."
        value={search}
        onChange={(e) => setSearch(e.target.value)}
        autoFocus
      />
      <div className="add-device-list">
        {filteredInstruments.length > 0 && (
          <>
            <div className="add-device-section-title">Instruments</div>
            {filteredInstruments.map((item) => (
              <button
                key={item.type}
                className="add-device-option"
                onClick={() => handleAddInstrument(item)}
              >
                <span className="add-device-option-name">{item.name}</span>
                <span className="add-device-option-tag tag-inst">Inst</span>
              </button>
            ))}
          </>
        )}
        {filteredEffects.length > 0 && (
          <>
            <div className="add-device-section-title">Effects</div>
            {filteredEffects.map((item) => (
              <button
                key={item.type}
                className="add-device-option"
                onClick={() => handleAddEffect(item)}
              >
                <span className="add-device-option-name">{item.name}</span>
                <span className="add-device-option-tag tag-fx">FX</span>
              </button>
            ))}
          </>
        )}
        {filteredInstruments.length === 0 && filteredEffects.length === 0 && (
          <div className="add-device-empty">No devices found</div>
        )}
      </div>
    </div>
  );
}

// Device Card Component
interface DeviceCardProps {
  device: Device;
  trackId: string;
}

function DeviceCard({ device, trackId }: DeviceCardProps) {
  const { removeDeviceFromTrack, setDeviceParameter, toggleDeviceEnabled } = useDAWStore();

  const isInstrument = device.type === 'instrument';
  const cardClass = `device-card ${isInstrument ? 'device-instrument' : 'device-effect'} ${!device.enabled ? 'device-disabled' : ''}`;

  const handleToggleEnabled = useCallback(() => {
    toggleDeviceEnabled(trackId, device.id);
  }, [trackId, device.id, toggleDeviceEnabled]);

  const [collapsed, setCollapsed] = useState(device.collapsed);

  const handleToggleCollapse = useCallback(() => {
    setCollapsed(prev => !prev);
  }, []);

  const handleRemove = useCallback(() => {
    removeDeviceFromTrack(trackId, device.id);
  }, [trackId, device.id, removeDeviceFromTrack]);

  const handleParamChange = useCallback((paramId: string, value: number) => {
    setDeviceParameter(trackId, device.id, paramId, value);
  }, [trackId, device.id, setDeviceParameter]);

  return (
    <div className={cardClass}>
      <div className="device-title-bar">
        <button
          className={`device-enable-btn ${device.enabled ? 'enabled' : ''}`}
          onClick={handleToggleEnabled}
          title={device.enabled ? 'Disable' : 'Enable'}
        >
          {device.enabled ? '\u25CF' : '\u25CB'}
        </button>
        <span className="device-type-icon">
          {isInstrument ? '\uD83C\uDFB9' : '\uD83C\uDF9B'}
        </span>
        <span className="device-name" title={device.name}>
          {device.name}
        </span>
        <button
          className="device-collapse-btn"
          onClick={handleToggleCollapse}
          title={collapsed ? 'Expand' : 'Collapse'}
        >
          {collapsed ? '\u25B6' : '\u25BC'}
        </button>
        <button
          className="device-remove-btn"
          onClick={handleRemove}
          title="Remove Device"
        >
          &#10005;
        </button>
      </div>
      {!collapsed && (
        <div className="device-params">
          {device.parameters.map((param) => (
            <Knob
              key={param.id}
              parameter={param}
              onChange={(val) => handleParamChange(param.id, val)}
            />
          ))}
          {device.parameters.length === 0 && (
            <div className="device-no-params">No parameters</div>
          )}
        </div>
      )}
    </div>
  );
}

// Main DeviceChain Component
export default function DeviceChain() {
  const {
    selectedTrackId, tracks, detailView,
    addDeviceToTrack,
  } = useDAWStore();

  const [showAddDropdown, setShowAddDropdown] = useState(false);

  const selectedTrack = tracks.find(t => t.id === selectedTrackId);

  const handleAddDevice = useCallback((device: Device) => {
    if (!selectedTrackId) return;
    addDeviceToTrack(selectedTrackId, device);
  }, [selectedTrackId, addDeviceToTrack]);

  if (detailView !== 'device') return null;

  return (
    <div className="device-chain-panel">
      {/* Track Info */}
      <div className="device-chain-track-info">
        {selectedTrack ? (
          <>
            <span
              className="device-chain-track-color"
              style={{ background: selectedTrack.color }}
            />
            <span className="device-chain-track-name">{selectedTrack.name}</span>
          </>
        ) : (
          <span className="device-chain-no-track">No track selected</span>
        )}
      </div>

      {/* Devices */}
      <div className="device-chain-devices">
        {selectedTrack && selectedTrack.devices.length > 0 ? (
          <>
            {selectedTrack.devices.map((device, index) => (
              <React.Fragment key={device.id}>
                {/* Drop zone indicator */}
                <div className="device-drop-zone" title="Drag device here">
                  <div className="drop-zone-line" />
                </div>
                <DeviceCard device={device} trackId={selectedTrack.id} />
              </React.Fragment>
            ))}
            <div className="device-drop-zone">
              <div className="drop-zone-line" />
            </div>
          </>
        ) : selectedTrack ? (
          <div className="device-chain-empty">
            <span className="empty-icon">{'\uD83C\uDFB9'}</span>
            <span className="empty-text">Drop an instrument or effect here</span>
            <span className="empty-sub">
              Use the browser or click + to add devices
            </span>
          </div>
        ) : (
          <div className="device-chain-empty">
            <span className="empty-text">Select a track to view its devices</span>
          </div>
        )}

        {/* Add Device Button */}
        {selectedTrack && (
          <div className="device-chain-add-wrapper">
            <button
              className="device-chain-add-btn"
              onClick={() => setShowAddDropdown(!showAddDropdown)}
              title="Add Device"
            >
              +
            </button>
            {showAddDropdown && (
              <AddDeviceDropdown
                onAdd={handleAddDevice}
                onClose={() => setShowAddDropdown(false)}
              />
            )}
          </div>
        )}
      </div>
    </div>
  );
}
