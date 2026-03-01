import { useRef } from 'react';
import './Toolbar.css';

export default function Toolbar({ onAddTrack, onImportFile, onOpenYoutube, onExport, mixerVisible, onToggleMixer }) {
  const fileInputRef = useRef(null);

  const handleFileSelect = (e) => {
    const file = e.target.files[0];
    if (file) {
      onImportFile(file);
      e.target.value = '';
    }
  };

  return (
    <div className="toolbar">
      <div className="toolbar-left">
        <span className="toolbar-logo">DAW Studio</span>
      </div>
      <div className="toolbar-center">
        <button className="toolbar-btn" onClick={onAddTrack} title="Add Track">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M7 1v12M1 7h12" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/></svg>
          Add Track
        </button>
        <button className="toolbar-btn" onClick={() => fileInputRef.current?.click()} title="Import Audio File">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M2 10v2h10v-2M7 2v7M4 6l3 3 3-3" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/></svg>
          Import
        </button>
        <input
          ref={fileInputRef}
          type="file"
          accept="audio/*"
          onChange={handleFileSelect}
          style={{ display: 'none' }}
        />
        <button className="toolbar-btn" onClick={onOpenYoutube} title="Download from YouTube">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><rect x="1" y="3" width="12" height="8" rx="2" stroke="currentColor" strokeWidth="1.2"/><path d="M5.5 5.5l3.5 2-3.5 2v-4z" fill="currentColor"/></svg>
          YouTube
        </button>
        <button className="toolbar-btn" onClick={onExport} title="Export Mix">
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M2 10v2h10v-2M7 9V2M4 5l3-3 3 3" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/></svg>
          Export
        </button>
      </div>
      <div className="toolbar-right">
        <button
          className={`toolbar-btn ${mixerVisible ? 'active' : ''}`}
          onClick={onToggleMixer}
          title="Toggle Mixer"
        >
          <svg width="14" height="14" viewBox="0 0 14 14" fill="none"><path d="M3 2v10M7 2v10M11 2v10" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/><circle cx="3" cy="8" r="1.5" fill="currentColor"/><circle cx="7" cy="5" r="1.5" fill="currentColor"/><circle cx="11" cy="9" r="1.5" fill="currentColor"/></svg>
          Mixer
        </button>
      </div>
    </div>
  );
}
