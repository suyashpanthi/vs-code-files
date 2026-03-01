import './Transport.css';

function formatTime(seconds) {
  const mins = Math.floor(seconds / 60);
  const secs = Math.floor(seconds % 60);
  const ms = Math.floor((seconds % 1) * 100);
  return `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}.${String(ms).padStart(2, '0')}`;
}

export default function Transport({
  transport, onPlay, onStop, onPause, onRecord,
  onToggleLoop, onBpmChange, onPositionChange
}) {
  return (
    <div className="transport">
      <div className="transport-left">
        <div className="transport-time" title="Current position">
          {formatTime(transport.position)}
        </div>
      </div>

      <div className="transport-controls">
        <button className="transport-btn" onClick={onStop} title="Stop (reset)">
          <svg width="16" height="16" viewBox="0 0 16 16" fill="none">
            <rect x="3" y="3" width="10" height="10" rx="1" fill="currentColor"/>
          </svg>
        </button>
        {transport.playing ? (
          <button className="transport-btn" onClick={onPause} title="Pause">
            <svg width="16" height="16" viewBox="0 0 16 16" fill="none">
              <rect x="3" y="2" width="3.5" height="12" rx="1" fill="currentColor"/>
              <rect x="9.5" y="2" width="3.5" height="12" rx="1" fill="currentColor"/>
            </svg>
          </button>
        ) : (
          <button className="transport-btn play" onClick={onPlay} title="Play">
            <svg width="18" height="18" viewBox="0 0 18 18" fill="none">
              <path d="M4 2.5l11.5 6.5L4 15.5V2.5z" fill="currentColor"/>
            </svg>
          </button>
        )}
        <button
          className={`transport-btn record ${transport.recording ? 'active' : ''}`}
          onClick={onRecord}
          title={transport.recording ? 'Stop Recording' : 'Record'}
        >
          <svg width="16" height="16" viewBox="0 0 16 16" fill="none">
            <circle cx="8" cy="8" r="5" fill="currentColor"/>
          </svg>
        </button>
        <button
          className={`transport-btn ${transport.loop ? 'loop-active' : ''}`}
          onClick={onToggleLoop}
          title="Loop"
        >
          <svg width="16" height="16" viewBox="0 0 16 16" fill="none">
            <path d="M12 4H5a3 3 0 000 6h1M4 12h7a3 3 0 000-6h-1" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
            <path d="M10 2l2 2-2 2M6 14l-2-2 2-2" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
          </svg>
        </button>
      </div>

      <div className="transport-right">
        <div className="transport-bpm">
          <label>BPM</label>
          <input
            type="number"
            value={transport.bpm}
            onChange={(e) => onBpmChange(Number(e.target.value))}
            min="20"
            max="300"
            className="bpm-input"
          />
        </div>
      </div>
    </div>
  );
}
