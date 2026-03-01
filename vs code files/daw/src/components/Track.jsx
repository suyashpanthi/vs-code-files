import { useRef, useEffect, useCallback, useState } from 'react';
import { drawWaveform } from '../audio/WaveformRenderer';
import './Track.css';

export default function Track({
  track, zoom, totalWidth, isSelected, selectedRegionId,
  onSelect, onSelectRegion, onUpdate, onRemove,
  onSplitRegion, onRemoveRegion, onMoveRegion,
  cursorPosition, onImportFile
}) {
  const [editingName, setEditingName] = useState(false);
  const [nameValue, setNameValue] = useState(track.name);
  const nameInputRef = useRef(null);

  useEffect(() => {
    if (editingName && nameInputRef.current) {
      nameInputRef.current.focus();
      nameInputRef.current.select();
    }
  }, [editingName]);

  const handleNameSubmit = () => {
    onUpdate({ name: nameValue || track.name });
    setEditingName(false);
  };

  const handleDrop = useCallback((e) => {
    e.preventDefault();
    e.stopPropagation();
    const files = e.dataTransfer.files;
    if (files.length > 0 && files[0].type.startsWith('audio/')) {
      onImportFile(files[0], track.id);
    }
  }, [track.id, onImportFile]);

  return (
    <div
      className={`track ${isSelected ? 'selected' : ''}`}
      onClick={onSelect}
      onDrop={handleDrop}
      onDragOver={(e) => { e.preventDefault(); e.dataTransfer.dropEffect = 'copy'; }}
    >
      {/* Track Header */}
      <div className="track-header" style={{ borderLeftColor: track.color }}>
        <div className="track-header-top">
          {editingName ? (
            <input
              ref={nameInputRef}
              className="track-name-input"
              value={nameValue}
              onChange={(e) => setNameValue(e.target.value)}
              onBlur={handleNameSubmit}
              onKeyDown={(e) => e.key === 'Enter' && handleNameSubmit()}
            />
          ) : (
            <span
              className="track-name"
              onDoubleClick={() => { setNameValue(track.name); setEditingName(true); }}
            >
              {track.name}
            </span>
          )}
          <button className="track-remove-btn" onClick={(e) => { e.stopPropagation(); onRemove(); }} title="Remove Track">
            <svg width="10" height="10" viewBox="0 0 10 10"><path d="M2 2l6 6M8 2l-6 6" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/></svg>
          </button>
        </div>
        <div className="track-header-controls">
          <button
            className={`track-ctrl-btn ${track.muted ? 'active-mute' : ''}`}
            onClick={(e) => { e.stopPropagation(); onUpdate({ muted: !track.muted }); }}
          >M</button>
          <button
            className={`track-ctrl-btn ${track.solo ? 'active-solo' : ''}`}
            onClick={(e) => { e.stopPropagation(); onUpdate({ solo: !track.solo }); }}
          >S</button>
          <input
            type="range"
            className="track-vol-slider"
            min="0"
            max="1"
            step="0.01"
            value={track.volume}
            onChange={(e) => { e.stopPropagation(); onUpdate({ volume: parseFloat(e.target.value) }); }}
            onClick={(e) => e.stopPropagation()}
            title={`Volume: ${Math.round(track.volume * 100)}%`}
          />
        </div>
      </div>

      {/* Track Lane (waveform regions) */}
      <div className="track-lane" style={{ width: totalWidth }}>
        {/* Grid lines */}
        <div className="track-grid" style={{ width: totalWidth }} />

        {track.regions.map(region => (
          <Region
            key={region.id}
            region={region}
            trackId={track.id}
            trackColor={track.color}
            zoom={zoom}
            isSelected={region.id === selectedRegionId}
            onSelect={() => onSelectRegion(region.id)}
            onSplit={(time) => onSplitRegion(region.id, time)}
            onRemove={() => onRemoveRegion(region.id)}
            onMove={(newStart) => onMoveRegion(track.id, track.id, region.id, newStart)}
          />
        ))}
      </div>
    </div>
  );
}

function Region({ region, trackId, trackColor, zoom, isSelected, onSelect, onSplit, onRemove, onMove }) {
  const canvasRef = useRef(null);
  const [dragging, setDragging] = useState(false);
  const dragStartRef = useRef({ x: 0, startTime: 0 });

  const width = region.buffer ? region.buffer.duration * zoom : 100;
  const left = region.startTime * zoom;

  useEffect(() => {
    if (canvasRef.current && region.buffer) {
      const canvas = canvasRef.current;
      const dpr = window.devicePixelRatio || 1;
      canvas.width = width * dpr;
      canvas.height = 60 * dpr;
      canvas.style.width = width + 'px';
      canvas.style.height = '60px';
      const ctx = canvas.getContext('2d');
      ctx.scale(dpr, dpr);
      drawWaveform(
        { getContext: () => ctx, width, height: 60 },
        region.buffer,
        trackColor
      );
    }
  }, [region.buffer, width, trackColor]);

  const handleMouseDown = (e) => {
    if (e.button !== 0) return;
    e.stopPropagation();
    onSelect();
    setDragging(true);
    dragStartRef.current = { x: e.clientX, startTime: region.startTime };

    const handleMouseMove = (e) => {
      const dx = e.clientX - dragStartRef.current.x;
      const dt = dx / zoom;
      onMove(dragStartRef.current.startTime + dt);
    };

    const handleMouseUp = () => {
      setDragging(false);
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('mouseup', handleMouseUp);
    };

    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', handleMouseUp);
  };

  const handleContextMenu = (e) => {
    e.preventDefault();
    e.stopPropagation();
    // Split at click position
    const rect = e.currentTarget.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const time = region.startTime + x / zoom;
    onSplit(time);
  };

  const handleDoubleClick = (e) => {
    e.stopPropagation();
    onRemove();
  };

  return (
    <div
      className={`region ${isSelected ? 'selected' : ''} ${dragging ? 'dragging' : ''}`}
      style={{
        left,
        width,
        borderColor: trackColor + '80',
        background: trackColor + '15',
      }}
      onMouseDown={handleMouseDown}
      onContextMenu={handleContextMenu}
      onDoubleClick={handleDoubleClick}
      title={`${region.name}\nRight-click: split | Double-click: delete`}
    >
      <div className="region-header" style={{ background: trackColor + '30' }}>
        <span className="region-name">{region.name}</span>
      </div>
      <canvas ref={canvasRef} className="region-canvas" />
    </div>
  );
}
