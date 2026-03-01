import { useState, useCallback, useRef } from 'react';
import { TRACK_COLOR_PALETTE } from '../audio/AudioEngine';

let nextTrackId = 1;
let nextRegionId = 1;

export default function useDAWState() {
  const [tracks, setTracks] = useState([]);
  const [transport, setTransport] = useState({
    playing: false,
    recording: false,
    position: 0,
    bpm: 120,
    loop: false,
    loopStart: 0,
    loopEnd: 8,
  });
  const [selectedTrackId, setSelectedTrackId] = useState(null);
  const [selectedRegionId, setSelectedRegionId] = useState(null);
  const [mixerVisible, setMixerVisible] = useState(true);
  const [zoom, setZoom] = useState(100); // pixels per second

  const addTrack = useCallback((name) => {
    const id = nextTrackId++;
    const color = TRACK_COLOR_PALETTE[(id - 1) % TRACK_COLOR_PALETTE.length];
    const track = {
      id,
      name: name || `Track ${id}`,
      regions: [],
      volume: 0.8,
      pan: 0,
      muted: false,
      solo: false,
      color,
      eqLow: 0,
      eqMid: 0,
      eqHigh: 0,
    };
    setTracks(prev => [...prev, track]);
    setSelectedTrackId(id);
    return id;
  }, []);

  const removeTrack = useCallback((trackId) => {
    setTracks(prev => prev.filter(t => t.id !== trackId));
    setSelectedTrackId(prev => prev === trackId ? null : prev);
  }, []);

  const updateTrack = useCallback((trackId, updates) => {
    setTracks(prev => prev.map(t => t.id === trackId ? { ...t, ...updates } : t));
  }, []);

  const addRegion = useCallback((trackId, audioBuffer, startTime = 0, name = '') => {
    const regionId = nextRegionId++;
    const region = {
      id: regionId,
      buffer: audioBuffer,
      startTime,
      name: name || `Region ${regionId}`,
    };
    setTracks(prev => prev.map(t =>
      t.id === trackId ? { ...t, regions: [...t.regions, region] } : t
    ));
    return regionId;
  }, []);

  const removeRegion = useCallback((trackId, regionId) => {
    setTracks(prev => prev.map(t =>
      t.id === trackId
        ? { ...t, regions: t.regions.filter(r => r.id !== regionId) }
        : t
    ));
    setSelectedRegionId(prev => prev === regionId ? null : prev);
  }, []);

  const updateRegion = useCallback((trackId, regionId, updates) => {
    setTracks(prev => prev.map(t =>
      t.id === trackId
        ? { ...t, regions: t.regions.map(r => r.id === regionId ? { ...r, ...updates } : r) }
        : t
    ));
  }, []);

  const splitRegion = useCallback((trackId, regionId, splitTime) => {
    setTracks(prev => prev.map(t => {
      if (t.id !== trackId) return t;
      const region = t.regions.find(r => r.id === regionId);
      if (!region || !region.buffer) return t;

      const relativeTime = splitTime - region.startTime;
      if (relativeTime <= 0 || relativeTime >= region.buffer.duration) return t;

      const sampleRate = region.buffer.sampleRate;
      const numChannels = region.buffer.numberOfChannels;
      const splitSample = Math.floor(relativeTime * sampleRate);

      // Create first half buffer
      const buf1Length = splitSample;
      const buf1 = new AudioContext().createBuffer(numChannels, buf1Length, sampleRate);
      for (let ch = 0; ch < numChannels; ch++) {
        const data = region.buffer.getChannelData(ch).slice(0, splitSample);
        buf1.getChannelData(ch).set(data);
      }

      // Create second half buffer
      const buf2Length = region.buffer.length - splitSample;
      const buf2 = new AudioContext().createBuffer(numChannels, buf2Length, sampleRate);
      for (let ch = 0; ch < numChannels; ch++) {
        const data = region.buffer.getChannelData(ch).slice(splitSample);
        buf2.getChannelData(ch).set(data);
      }

      const newRegions = t.regions.filter(r => r.id !== regionId);
      const id1 = nextRegionId++;
      const id2 = nextRegionId++;
      newRegions.push(
        { id: id1, buffer: buf1, startTime: region.startTime, name: region.name + ' L' },
        { id: id2, buffer: buf2, startTime: splitTime, name: region.name + ' R' }
      );
      return { ...t, regions: newRegions };
    }));
  }, []);

  const moveRegion = useCallback((fromTrackId, toTrackId, regionId, newStartTime) => {
    let movedRegion = null;
    setTracks(prev => {
      const updated = prev.map(t => {
        if (t.id === fromTrackId) {
          const region = t.regions.find(r => r.id === regionId);
          if (region) {
            movedRegion = { ...region, startTime: Math.max(0, newStartTime) };
          }
          return { ...t, regions: t.regions.filter(r => r.id !== regionId) };
        }
        return t;
      });
      if (movedRegion) {
        return updated.map(t =>
          t.id === toTrackId ? { ...t, regions: [...t.regions, movedRegion] } : t
        );
      }
      return updated;
    });
  }, []);

  const setPosition = useCallback((pos) => {
    setTransport(prev => ({ ...prev, position: Math.max(0, pos) }));
  }, []);

  const setPlaying = useCallback((playing) => {
    setTransport(prev => ({ ...prev, playing }));
  }, []);

  const setRecording = useCallback((recording) => {
    setTransport(prev => ({ ...prev, recording }));
  }, []);

  const setBpm = useCallback((bpm) => {
    setTransport(prev => ({ ...prev, bpm: Math.max(20, Math.min(300, bpm)) }));
  }, []);

  const toggleLoop = useCallback(() => {
    setTransport(prev => ({ ...prev, loop: !prev.loop }));
  }, []);

  const getTotalDuration = useCallback(() => {
    let maxEnd = 10;
    tracks.forEach(t => {
      t.regions.forEach(r => {
        if (r.buffer) {
          const end = r.startTime + r.buffer.duration;
          if (end > maxEnd) maxEnd = end;
        }
      });
    });
    return maxEnd + 5;
  }, [tracks]);

  return {
    tracks, setTracks,
    transport,
    selectedTrackId, setSelectedTrackId,
    selectedRegionId, setSelectedRegionId,
    mixerVisible, setMixerVisible,
    zoom, setZoom,
    addTrack, removeTrack, updateTrack,
    addRegion, removeRegion, updateRegion,
    splitRegion, moveRegion,
    setPosition, setPlaying, setRecording,
    setBpm, toggleLoop, getTotalDuration,
  };
}
