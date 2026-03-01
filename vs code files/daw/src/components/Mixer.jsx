import MixerChannel from './MixerChannel';
import './Mixer.css';

export default function Mixer({ tracks, onUpdateTrack, selectedTrackId, onSelectTrack }) {
  return (
    <div className="mixer">
      <div className="mixer-label">MIXER</div>
      <div className="mixer-channels">
        {tracks.map(track => (
          <MixerChannel
            key={track.id}
            track={track}
            isSelected={track.id === selectedTrackId}
            onSelect={() => onSelectTrack(track.id)}
            onUpdate={(updates) => onUpdateTrack(track.id, updates)}
          />
        ))}
        {tracks.length === 0 && (
          <div className="mixer-empty">No tracks</div>
        )}
        {/* Master channel */}
        <div className="mixer-channel master">
          <div className="channel-label">Master</div>
          <div className="channel-fader-container">
            <input
              type="range"
              className="channel-fader"
              min="0"
              max="1"
              step="0.01"
              defaultValue="0.8"
              orient="vertical"
            />
            <div className="channel-meter">
              <div className="meter-fill" style={{ height: '60%' }} />
            </div>
          </div>
          <div className="channel-value">0.0 dB</div>
        </div>
      </div>
    </div>
  );
}
