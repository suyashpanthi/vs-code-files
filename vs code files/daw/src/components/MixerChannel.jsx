import './MixerChannel.css';

export default function MixerChannel({ track, isSelected, onSelect, onUpdate }) {
  const dbValue = track.volume > 0
    ? (20 * Math.log10(track.volume)).toFixed(1)
    : '-inf';

  return (
    <div
      className={`mixer-channel ${isSelected ? 'selected' : ''}`}
      onClick={onSelect}
      style={{ '--ch-color': track.color }}
    >
      <div className="channel-label" style={{ color: track.color }}>
        {track.name}
      </div>

      <div className="channel-eq">
        <div className="eq-knob-group">
          <label>H</label>
          <input
            type="range"
            className="eq-knob"
            min="-12"
            max="12"
            step="0.5"
            value={track.eqHigh}
            onChange={(e) => onUpdate({ eqHigh: parseFloat(e.target.value) })}
            onClick={(e) => e.stopPropagation()}
          />
        </div>
        <div className="eq-knob-group">
          <label>M</label>
          <input
            type="range"
            className="eq-knob"
            min="-12"
            max="12"
            step="0.5"
            value={track.eqMid}
            onChange={(e) => onUpdate({ eqMid: parseFloat(e.target.value) })}
            onClick={(e) => e.stopPropagation()}
          />
        </div>
        <div className="eq-knob-group">
          <label>L</label>
          <input
            type="range"
            className="eq-knob"
            min="-12"
            max="12"
            step="0.5"
            value={track.eqLow}
            onChange={(e) => onUpdate({ eqLow: parseFloat(e.target.value) })}
            onClick={(e) => e.stopPropagation()}
          />
        </div>
      </div>

      <div className="channel-pan-container">
        <label>Pan</label>
        <input
          type="range"
          className="channel-pan"
          min="-1"
          max="1"
          step="0.01"
          value={track.pan}
          onChange={(e) => onUpdate({ pan: parseFloat(e.target.value) })}
          onClick={(e) => e.stopPropagation()}
        />
      </div>

      <div className="channel-fader-container">
        <input
          type="range"
          className="channel-fader"
          min="0"
          max="1"
          step="0.01"
          value={track.volume}
          onChange={(e) => onUpdate({ volume: parseFloat(e.target.value) })}
          onClick={(e) => e.stopPropagation()}
          orient="vertical"
        />
        <div className="channel-meter">
          <div
            className="meter-fill"
            style={{
              height: `${track.muted ? 0 : track.volume * 100}%`,
              background: track.color,
            }}
          />
        </div>
      </div>

      <div className="channel-value">{dbValue} dB</div>

      <div className="channel-buttons">
        <button
          className={`ch-btn ${track.muted ? 'active-mute' : ''}`}
          onClick={(e) => { e.stopPropagation(); onUpdate({ muted: !track.muted }); }}
        >M</button>
        <button
          className={`ch-btn ${track.solo ? 'active-solo' : ''}`}
          onClick={(e) => { e.stopPropagation(); onUpdate({ solo: !track.solo }); }}
        >S</button>
      </div>

      <div className="channel-color-bar" style={{ background: track.color }} />
    </div>
  );
}
