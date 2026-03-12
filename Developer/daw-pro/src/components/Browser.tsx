import React, { useState, useMemo, useCallback } from 'react';
import { v4 as uuid } from 'uuid';
import { useDAWStore } from '../store';
import { audioEngine, getEffectParameters, getSynthParameters, SynthType, EffectType } from '../engine/AudioEngine';
import type { Device } from '../types';
import './Browser.css';

interface BrowserItem {
  id: string;
  name: string;
  icon: string;
  type: 'instrument' | 'audioEffect';
  subType: string;
  category: string;
}

const SYNTH_ITEMS: BrowserItem[] = [
  { id: 'poly', name: 'Poly Synth', icon: '\uD83C\uDFB9', type: 'instrument', subType: 'poly', category: 'instruments' },
  { id: 'fm', name: 'FM Synth', icon: '\uD83C\uDFB9', type: 'instrument', subType: 'fm', category: 'instruments' },
  { id: 'am', name: 'AM Synth', icon: '\uD83C\uDFB9', type: 'instrument', subType: 'am', category: 'instruments' },
  { id: 'mono', name: 'Mono Synth', icon: '\uD83C\uDFB9', type: 'instrument', subType: 'mono', category: 'instruments' },
  { id: 'membrane', name: 'Membrane', icon: '\uD83E\uDD41', type: 'instrument', subType: 'membrane', category: 'instruments' },
  { id: 'metal', name: 'Metal', icon: '\uD83D\uDD14', type: 'instrument', subType: 'metal', category: 'instruments' },
  { id: 'pluck', name: 'Pluck', icon: '\uD83C\uDFB8', type: 'instrument', subType: 'pluck', category: 'instruments' },
  { id: 'noise', name: 'Noise', icon: '\uD83C\uDF2A', type: 'instrument', subType: 'noise', category: 'instruments' },
  { id: 'duo', name: 'Duo', icon: '\uD83C\uDFB9', type: 'instrument', subType: 'duo', category: 'instruments' },
  { id: 'wavetable', name: 'Wavetable', icon: '\u3030', type: 'instrument', subType: 'wavetable', category: 'instruments' },
];

const EFFECT_ITEMS: BrowserItem[] = [
  { id: 'reverb', name: 'Reverb', icon: '\uD83C\uDFDB', type: 'audioEffect', subType: 'reverb', category: 'effects' },
  { id: 'delay', name: 'Delay', icon: '\u23F1', type: 'audioEffect', subType: 'delay', category: 'effects' },
  { id: 'chorus', name: 'Chorus', icon: '\uD83C\uDFA4', type: 'audioEffect', subType: 'chorus', category: 'effects' },
  { id: 'phaser', name: 'Phaser', icon: '\uD83C\uDF00', type: 'audioEffect', subType: 'phaser', category: 'effects' },
  { id: 'distortion', name: 'Distortion', icon: '\u26A1', type: 'audioEffect', subType: 'distortion', category: 'effects' },
  { id: 'compressor', name: 'Compressor', icon: '\uD83D\uDCCA', type: 'audioEffect', subType: 'compressor', category: 'effects' },
  { id: 'eq3', name: 'EQ3', icon: '\uD83C\uDF9B', type: 'audioEffect', subType: 'eq3', category: 'effects' },
  { id: 'filter', name: 'Filter', icon: '\uD83C\uDF9A', type: 'audioEffect', subType: 'filter', category: 'effects' },
  { id: 'tremolo', name: 'Tremolo', icon: '\u223F', type: 'audioEffect', subType: 'tremolo', category: 'effects' },
  { id: 'vibrato', name: 'Vibrato', icon: '\u223F', type: 'audioEffect', subType: 'vibrato', category: 'effects' },
  { id: 'bitcrusher', name: 'BitCrusher', icon: '\uD83D\uDC7E', type: 'audioEffect', subType: 'bitcrusher', category: 'effects' },
  { id: 'chebyshev', name: 'Chebyshev', icon: '\uD83D\uDD38', type: 'audioEffect', subType: 'chebyshev', category: 'effects' },
  { id: 'freeverb', name: 'FreeVerb', icon: '\uD83C\uDFDB', type: 'audioEffect', subType: 'freeverb', category: 'effects' },
  { id: 'pingpong', name: 'Ping Pong Delay', icon: '\uD83C\uDFD3', type: 'audioEffect', subType: 'pingpong', category: 'effects' },
  { id: 'autowah', name: 'AutoWah', icon: '\uD83D\uDC44', type: 'audioEffect', subType: 'autowah', category: 'effects' },
  { id: 'pitchshift', name: 'Pitch Shift', icon: '\u2195', type: 'audioEffect', subType: 'pitchshift', category: 'effects' },
];

const DRUM_KITS = [
  { id: 'kit-808', name: '808 Kit', icon: '\uD83E\uDD41' },
  { id: 'kit-909', name: '909 Kit', icon: '\uD83E\uDD41' },
  { id: 'kit-acoustic', name: 'Acoustic Kit', icon: '\uD83E\uDD41' },
  { id: 'kit-electronic', name: 'Electronic Kit', icon: '\uD83E\uDD41' },
  { id: 'kit-hiphop', name: 'Hip Hop Kit', icon: '\uD83E\uDD41' },
  { id: 'kit-jazz', name: 'Jazz Kit', icon: '\uD83E\uDD41' },
];

interface CategoryDef {
  id: string;
  name: string;
  icon: string;
}

const CATEGORIES: CategoryDef[] = [
  { id: 'instruments', name: 'Instruments', icon: '\uD83C\uDFB9' },
  { id: 'effects', name: 'Effects', icon: '\uD83C\uDF9B' },
  { id: 'samples', name: 'Samples', icon: '\uD83D\uDD0A' },
  { id: 'drums', name: 'Drums', icon: '\uD83E\uDD41' },
  { id: 'clips', name: 'Clips', icon: '\uD83D\uDCCE' },
  { id: 'plugins', name: 'Plugins', icon: '\uD83D\uDD0C' },
];

export default function Browser() {
  const {
    showBrowser, toggleBrowser,
    browserCategory, setBrowserCategory,
    browserSearchQuery, setBrowserSearch,
    selectedTrackId, tracks,
    addDeviceToTrack, setDetailView,
  } = useDAWStore();

  const [collapsedSections, setCollapsedSections] = useState<Set<string>>(new Set());
  const [previewingId, setPreviewingId] = useState<string | null>(null);

  const toggleSection = useCallback((sectionId: string) => {
    setCollapsedSections(prev => {
      const next = new Set(prev);
      if (next.has(sectionId)) {
        next.delete(sectionId);
      } else {
        next.add(sectionId);
      }
      return next;
    });
  }, []);

  const filteredItems = useMemo(() => {
    const query = browserSearchQuery.toLowerCase().trim();
    let items: BrowserItem[] = [];

    if (browserCategory === 'instruments') {
      items = SYNTH_ITEMS;
    } else if (browserCategory === 'effects') {
      items = EFFECT_ITEMS;
    }

    if (query) {
      items = items.filter(item => item.name.toLowerCase().includes(query));
    }

    return items;
  }, [browserCategory, browserSearchQuery]);

  const filteredDrumKits = useMemo(() => {
    const query = browserSearchQuery.toLowerCase().trim();
    if (!query) return DRUM_KITS;
    return DRUM_KITS.filter(kit => kit.name.toLowerCase().includes(query));
  }, [browserSearchQuery]);

  const handleAddDevice = useCallback((item: BrowserItem) => {
    if (!selectedTrackId) return;

    let device: Device;
    if (item.type === 'instrument') {
      const params = getSynthParameters(item.subType as SynthType);
      device = {
        id: uuid(),
        name: item.name,
        type: 'instrument',
        subType: item.subType as any,
        enabled: true,
        collapsed: false,
        parameters: params,
      };
    } else {
      const params = getEffectParameters(item.subType as EffectType);
      device = {
        id: uuid(),
        name: item.name,
        type: 'audioEffect',
        subType: item.subType as any,
        enabled: true,
        collapsed: false,
        parameters: params,
      };
    }

    addDeviceToTrack(selectedTrackId, device);
    setDetailView('device');
  }, [selectedTrackId, addDeviceToTrack, setDetailView]);

  const handlePreview = useCallback(async (item: BrowserItem) => {
    if (item.type !== 'instrument') return;
    try {
      await audioEngine.init();
    } catch {
      // already initialized
    }
    // Play a short preview note using a temporary mechanism
    // Use the selected track if available, otherwise just trigger on any MIDI track
    const targetTrackId = selectedTrackId;
    if (!targetTrackId) return;

    const track = tracks.find(t => t.id === targetTrackId);
    if (!track) return;

    setPreviewingId(item.id);
    // Play a C4 note as preview
    audioEngine.playNote(targetTrackId, 60, 0.7, '8n');
    setTimeout(() => setPreviewingId(null), 500);
  }, [selectedTrackId, tracks]);

  if (!showBrowser) return null;

  const selectedTrack = tracks.find(t => t.id === selectedTrackId);

  return (
    <div className="browser-panel">
      <div className="browser-header">
        <span className="browser-title">Browser</span>
        <button className="browser-close-btn" onClick={toggleBrowser} title="Close Browser">
          &#10005;
        </button>
      </div>

      <div className="browser-search">
        <span className="search-icon">&#128269;</span>
        <input
          type="text"
          className="browser-search-input"
          placeholder="Search..."
          value={browserSearchQuery}
          onChange={(e) => setBrowserSearch(e.target.value)}
        />
        {browserSearchQuery && (
          <button className="search-clear" onClick={() => setBrowserSearch('')}>
            &#10005;
          </button>
        )}
      </div>

      <div className="browser-categories">
        {CATEGORIES.map((cat) => (
          <button
            key={cat.id}
            className={`category-btn ${browserCategory === cat.id ? 'active' : ''}`}
            onClick={() => setBrowserCategory(cat.id)}
          >
            <span className="category-icon">{cat.icon}</span>
            <span className="category-name">{cat.name}</span>
          </button>
        ))}
      </div>

      <div className="browser-content">
        {browserCategory === 'instruments' && (
          <div className="browser-list">
            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('synths')}
              >
                <span className="section-arrow">{collapsedSections.has('synths') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">Synthesizers</span>
                <span className="section-count">{filteredItems.length}</span>
              </button>
              {!collapsedSections.has('synths') && (
                <div className="section-items">
                  {filteredItems.map((item) => (
                    <div
                      key={item.id}
                      className="browser-item"
                      onDoubleClick={() => handleAddDevice(item)}
                      title={`Double-click to add ${item.name} to selected track`}
                    >
                      <span className="item-icon">{item.icon}</span>
                      <span className="item-name">{item.name}</span>
                      <span className="item-tag tag-instrument">Inst</span>
                      <button
                        className={`item-preview-btn ${previewingId === item.id ? 'previewing' : ''}`}
                        onClick={(e) => {
                          e.stopPropagation();
                          handlePreview(item);
                        }}
                        title="Preview"
                      >
                        &#9654;
                      </button>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </div>
        )}

        {browserCategory === 'effects' && (
          <div className="browser-list">
            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('fx-dynamics')}
              >
                <span className="section-arrow">{collapsedSections.has('fx-dynamics') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">Dynamics</span>
              </button>
              {!collapsedSections.has('fx-dynamics') && (
                <div className="section-items">
                  {filteredItems.filter(i => ['compressor'].includes(i.subType)).map((item) => (
                    <div
                      key={item.id}
                      className="browser-item"
                      onDoubleClick={() => handleAddDevice(item)}
                      title={`Double-click to add ${item.name}`}
                    >
                      <span className="item-icon">{item.icon}</span>
                      <span className="item-name">{item.name}</span>
                      <span className="item-tag tag-effect">FX</span>
                    </div>
                  ))}
                </div>
              )}
            </div>

            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('fx-eq')}
              >
                <span className="section-arrow">{collapsedSections.has('fx-eq') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">EQ & Filter</span>
              </button>
              {!collapsedSections.has('fx-eq') && (
                <div className="section-items">
                  {filteredItems.filter(i => ['eq3', 'filter'].includes(i.subType)).map((item) => (
                    <div
                      key={item.id}
                      className="browser-item"
                      onDoubleClick={() => handleAddDevice(item)}
                      title={`Double-click to add ${item.name}`}
                    >
                      <span className="item-icon">{item.icon}</span>
                      <span className="item-name">{item.name}</span>
                      <span className="item-tag tag-effect">FX</span>
                    </div>
                  ))}
                </div>
              )}
            </div>

            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('fx-time')}
              >
                <span className="section-arrow">{collapsedSections.has('fx-time') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">Time-Based</span>
              </button>
              {!collapsedSections.has('fx-time') && (
                <div className="section-items">
                  {filteredItems.filter(i => ['reverb', 'delay', 'freeverb', 'pingpong', 'feedbackdelay'].includes(i.subType)).map((item) => (
                    <div
                      key={item.id}
                      className="browser-item"
                      onDoubleClick={() => handleAddDevice(item)}
                      title={`Double-click to add ${item.name}`}
                    >
                      <span className="item-icon">{item.icon}</span>
                      <span className="item-name">{item.name}</span>
                      <span className="item-tag tag-effect">FX</span>
                    </div>
                  ))}
                </div>
              )}
            </div>

            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('fx-mod')}
              >
                <span className="section-arrow">{collapsedSections.has('fx-mod') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">Modulation</span>
              </button>
              {!collapsedSections.has('fx-mod') && (
                <div className="section-items">
                  {filteredItems.filter(i => ['chorus', 'phaser', 'tremolo', 'vibrato', 'autowah'].includes(i.subType)).map((item) => (
                    <div
                      key={item.id}
                      className="browser-item"
                      onDoubleClick={() => handleAddDevice(item)}
                      title={`Double-click to add ${item.name}`}
                    >
                      <span className="item-icon">{item.icon}</span>
                      <span className="item-name">{item.name}</span>
                      <span className="item-tag tag-effect">FX</span>
                    </div>
                  ))}
                </div>
              )}
            </div>

            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('fx-dist')}
              >
                <span className="section-arrow">{collapsedSections.has('fx-dist') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">Distortion & Saturation</span>
              </button>
              {!collapsedSections.has('fx-dist') && (
                <div className="section-items">
                  {filteredItems.filter(i => ['distortion', 'bitcrusher', 'chebyshev'].includes(i.subType)).map((item) => (
                    <div
                      key={item.id}
                      className="browser-item"
                      onDoubleClick={() => handleAddDevice(item)}
                      title={`Double-click to add ${item.name}`}
                    >
                      <span className="item-icon">{item.icon}</span>
                      <span className="item-name">{item.name}</span>
                      <span className="item-tag tag-effect">FX</span>
                    </div>
                  ))}
                </div>
              )}
            </div>

            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('fx-pitch')}
              >
                <span className="section-arrow">{collapsedSections.has('fx-pitch') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">Pitch</span>
              </button>
              {!collapsedSections.has('fx-pitch') && (
                <div className="section-items">
                  {filteredItems.filter(i => ['pitchshift'].includes(i.subType)).map((item) => (
                    <div
                      key={item.id}
                      className="browser-item"
                      onDoubleClick={() => handleAddDevice(item)}
                      title={`Double-click to add ${item.name}`}
                    >
                      <span className="item-icon">{item.icon}</span>
                      <span className="item-name">{item.name}</span>
                      <span className="item-tag tag-effect">FX</span>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </div>
        )}

        {browserCategory === 'drums' && (
          <div className="browser-list">
            <div className="list-section">
              <button
                className="section-header"
                onClick={() => toggleSection('drum-kits')}
              >
                <span className="section-arrow">{collapsedSections.has('drum-kits') ? '\u25B6' : '\u25BC'}</span>
                <span className="section-title">Drum Kits</span>
                <span className="section-count">{filteredDrumKits.length}</span>
              </button>
              {!collapsedSections.has('drum-kits') && (
                <div className="section-items">
                  {filteredDrumKits.map((kit) => (
                    <div key={kit.id} className="browser-item browser-item-placeholder">
                      <span className="item-icon">{kit.icon}</span>
                      <span className="item-name">{kit.name}</span>
                      <span className="item-tag tag-drums">Kit</span>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </div>
        )}

        {browserCategory === 'samples' && (
          <div className="browser-placeholder">
            <span className="placeholder-icon">{'\uD83D\uDD0A'}</span>
            <span className="placeholder-text">Sample Browser</span>
            <span className="placeholder-sub">Drag audio files here or browse your library</span>
          </div>
        )}

        {browserCategory === 'clips' && (
          <div className="browser-placeholder">
            <span className="placeholder-icon">{'\uD83D\uDCCE'}</span>
            <span className="placeholder-text">Clip Browser</span>
            <span className="placeholder-sub">Browse MIDI and audio clips</span>
          </div>
        )}

        {browserCategory === 'plugins' && (
          <div className="browser-placeholder">
            <span className="placeholder-icon">{'\uD83D\uDD0C'}</span>
            <span className="placeholder-text">Plugin Browser</span>
            <span className="placeholder-sub">VST3 / AU plugins will appear here</span>
          </div>
        )}
      </div>

      {selectedTrack && (
        <div className="browser-footer">
          <span className="footer-track-indicator" style={{ background: selectedTrack.color }} />
          <span className="footer-text">Target: {selectedTrack.name}</span>
        </div>
      )}
    </div>
  );
}
