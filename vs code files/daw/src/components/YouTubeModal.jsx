import { useState } from 'react';
import './YouTubeModal.css';

export default function YouTubeModal({ onDownload, onClose }) {
  const [url, setUrl] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const [progress, setProgress] = useState('');

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!url.trim()) return;

    setLoading(true);
    setError('');
    setProgress('Downloading audio...');

    try {
      await onDownload(url.trim());
    } catch (err) {
      setError(err.message || 'Download failed. Make sure the backend is running.');
    } finally {
      setLoading(false);
      setProgress('');
    }
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal" onClick={(e) => e.stopPropagation()}>
        <div className="modal-header">
          <h3>Import from YouTube</h3>
          <button className="modal-close" onClick={onClose}>
            <svg width="14" height="14" viewBox="0 0 14 14"><path d="M3 3l8 8M11 3l-8 8" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/></svg>
          </button>
        </div>
        <form onSubmit={handleSubmit}>
          <div className="modal-body">
            <label htmlFor="yt-url">YouTube URL</label>
            <input
              id="yt-url"
              type="text"
              className="modal-input"
              placeholder="https://www.youtube.com/watch?v=..."
              value={url}
              onChange={(e) => setUrl(e.target.value)}
              disabled={loading}
              autoFocus
            />
            {progress && <div className="modal-progress">{progress}</div>}
            {error && <div className="modal-error">{error}</div>}
            <p className="modal-hint">
              Requires the backend server running at localhost:5000
            </p>
          </div>
          <div className="modal-footer">
            <button type="button" className="modal-btn cancel" onClick={onClose} disabled={loading}>
              Cancel
            </button>
            <button type="submit" className="modal-btn primary" disabled={loading || !url.trim()}>
              {loading ? 'Downloading...' : 'Download & Import'}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}
