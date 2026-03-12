import { create } from 'zustand';
import { v4 as uuid } from 'uuid';
import type {
  Track, Clip, Scene, Device, MidiNote, AutomationLane,
  ViewMode, DetailView, TrackType, TRACK_COLORS, DeviceParameter,
} from './types';

const COLORS = [
  '#e74c3c', '#e67e22', '#f1c40f', '#2ecc71',
  '#1abc9c', '#3498db', '#9b59b6', '#e91e63',
  '#00bcd4', '#8bc34a', '#ff9800', '#795548',
];

function createDefaultTrack(type: TrackType, index: number): Track {
  const names: Record<TrackType, string> = {
    audio: `Audio ${index + 1}`,
    midi: `MIDI ${index + 1}`,
    return: `Return ${String.fromCharCode(65 + index)}`,
    master: 'Master',
    group: `Group ${index + 1}`,
  };
  return {
    id: uuid(),
    name: names[type],
    type,
    color: COLORS[index % COLORS.length],
    volume: 0.8,
    pan: 0,
    mute: false,
    solo: false,
    armed: false,
    monitorInput: false,
    devices: [],
    sends: [],
    clips: {},
    arrangementClips: [],
    automation: [],
    inputSource: 'default',
    outputTarget: 'master',
    height: 80,
    collapsed: false,
    children: [],
    freeze: false,
    meterLeft: 0,
    meterRight: 0,
    activeClipId: null,
  };
}

function createDefaultClip(trackId: string, type: 'audio' | 'midi', sceneIndex: number, color: string): Clip {
  return {
    id: uuid(),
    trackId,
    name: type === 'midi' ? 'MIDI Clip' : 'Audio Clip',
    color,
    type,
    startTime: sceneIndex * 4,
    duration: 4,
    loopEnabled: true,
    loopStart: 0,
    loopEnd: 4,
    launchMode: 'trigger',
    notes: type === 'midi' ? [] : undefined,
    gain: 0,
    transpose: 0,
    detune: 0,
    reversed: false,
    warpEnabled: type === 'audio',
    warpMode: 'beats',
    isPlaying: false,
    isRecording: false,
    automation: [],
  };
}

function createDefaultScene(index: number): Scene {
  return {
    id: uuid(),
    name: `Scene ${index + 1}`,
    color: '#555555',
  };
}

export interface DAWState {
  // Project
  projectName: string;
  bpm: number;
  timeSignature: [number, number];
  sampleRate: number;

  // Transport
  isPlaying: boolean;
  isRecording: boolean;
  loopEnabled: boolean;
  loopStart: number;
  loopEnd: number;
  currentBeat: number;
  metronomeEnabled: boolean;
  countIn: number;
  preRoll: boolean;
  followPlayhead: boolean;

  // Tracks
  tracks: Track[];
  masterTrack: Track;
  returnTracks: Track[];
  selectedTrackId: string | null;

  // Scenes
  scenes: Scene[];
  selectedSceneIndex: number;

  // UI State
  activeView: ViewMode;
  showBrowser: boolean;
  showDetail: boolean;
  showMixer: boolean;
  detailView: DetailView;
  selectedClipId: string | null;
  horizontalZoom: number;
  verticalZoom: number;
  scrollX: number;
  scrollY: number;
  drawMode: boolean;
  snapToGrid: boolean;
  gridSize: number;
  soloExclusive: boolean;

  // Browser
  browserCategory: string;
  browserSearchQuery: string;

  // Clipboard
  clipboardClip: Clip | null;
  clipboardNotes: MidiNote[] | null;

  // Undo
  undoStack: string[];
  redoStack: string[];

  // Actions
  setProjectName: (name: string) => void;
  setBPM: (bpm: number) => void;
  setTimeSignature: (ts: [number, number]) => void;

  // Transport actions
  play: () => void;
  stop: () => void;
  togglePlay: () => void;
  record: () => void;
  setCurrentBeat: (beat: number) => void;
  toggleLoop: () => void;
  setLoopRegion: (start: number, end: number) => void;
  toggleMetronome: () => void;
  toggleFollowPlayhead: () => void;

  // Track actions
  addTrack: (type: TrackType) => void;
  removeTrack: (id: string) => void;
  duplicateTrack: (id: string) => void;
  selectTrack: (id: string | null) => void;
  setTrackVolume: (id: string, volume: number) => void;
  setTrackPan: (id: string, pan: number) => void;
  toggleTrackMute: (id: string) => void;
  toggleTrackSolo: (id: string) => void;
  toggleTrackArm: (id: string) => void;
  setTrackColor: (id: string, color: string) => void;
  renameTrack: (id: string, name: string) => void;
  reorderTracks: (fromIndex: number, toIndex: number) => void;
  setTrackMeter: (id: string, left: number, right: number) => void;
  setMasterMeter: (left: number, right: number) => void;
  addDeviceToTrack: (trackId: string, device: Device) => void;
  removeDeviceFromTrack: (trackId: string, deviceId: string) => void;
  setDeviceParameter: (trackId: string, deviceId: string, paramId: string, value: number) => void;
  toggleDeviceEnabled: (trackId: string, deviceId: string) => void;

  // Clip actions
  createClip: (trackId: string, sceneIndex: number) => void;
  deleteClip: (trackId: string, sceneIndex: number) => void;
  setClipPlaying: (trackId: string, clipId: string, playing: boolean) => void;
  launchClip: (trackId: string, sceneIndex: number) => void;
  stopTrackClip: (trackId: string) => void;
  launchScene: (sceneIndex: number) => void;
  stopAllClips: () => void;
  selectClip: (clipId: string | null) => void;
  updateClip: (clipId: string, updates: Partial<Clip>) => void;

  // Arrangement clip actions
  addArrangementClip: (trackId: string, clip: Clip) => void;
  moveArrangementClip: (trackId: string, clipId: string, newStart: number) => void;
  resizeArrangementClip: (trackId: string, clipId: string, newDuration: number) => void;
  splitArrangementClip: (trackId: string, clipId: string, splitPoint: number) => void;
  deleteArrangementClip: (trackId: string, clipId: string) => void;

  // MIDI actions
  addNote: (clipId: string, note: MidiNote) => void;
  removeNote: (clipId: string, noteId: string) => void;
  updateNote: (clipId: string, noteId: string, updates: Partial<MidiNote>) => void;
  moveNote: (clipId: string, noteId: string, pitch: number, startTime: number) => void;
  resizeNote: (clipId: string, noteId: string, duration: number) => void;

  // Scene actions
  addScene: () => void;
  removeScene: (index: number) => void;
  renameScene: (index: number, name: string) => void;
  selectScene: (index: number) => void;

  // View actions
  toggleView: () => void;
  setActiveView: (view: ViewMode) => void;
  toggleBrowser: () => void;
  toggleDetail: () => void;
  toggleMixer: () => void;
  setDetailView: (view: DetailView) => void;
  setHorizontalZoom: (zoom: number) => void;
  setVerticalZoom: (zoom: number) => void;
  setScrollX: (x: number) => void;
  setScrollY: (y: number) => void;
  toggleDrawMode: () => void;
  toggleSnapToGrid: () => void;
  setGridSize: (size: number) => void;
  setBrowserCategory: (cat: string) => void;
  setBrowserSearch: (query: string) => void;
}

function findClipInAllTracks(tracks: Track[], clipId: string): { track: Track; clip: Clip; sceneIndex?: number; arrIndex?: number } | null {
  for (const track of tracks) {
    for (const [si, clip] of Object.entries(track.clips)) {
      if (clip.id === clipId) return { track, clip, sceneIndex: Number(si) };
    }
    const arrIdx = track.arrangementClips.findIndex(c => c.id === clipId);
    if (arrIdx >= 0) return { track, clip: track.arrangementClips[arrIdx], arrIndex: arrIdx };
  }
  return null;
}

export const useDAWStore = create<DAWState>((set, get) => {
  const initialTracks: Track[] = [
    { ...createDefaultTrack('midi', 0), name: '1 MIDI', devices: [] },
    { ...createDefaultTrack('midi', 1), name: '2 MIDI', devices: [] },
    { ...createDefaultTrack('audio', 2), name: '3 Audio', devices: [] },
    { ...createDefaultTrack('audio', 3), name: '4 Audio', devices: [] },
  ];

  const masterTrack: Track = {
    ...createDefaultTrack('master', 0),
    name: 'Master',
    color: '#ff9800',
    volume: 0.85,
  };

  const returnTracks: Track[] = [
    { ...createDefaultTrack('return', 0), name: 'A Reverb', color: '#00bcd4' },
    { ...createDefaultTrack('return', 1), name: 'B Delay', color: '#8bc34a' },
  ];

  const scenes: Scene[] = Array.from({ length: 8 }, (_, i) => createDefaultScene(i));

  return {
    projectName: 'Untitled Project',
    bpm: 120,
    timeSignature: [4, 4] as [number, number],
    sampleRate: 44100,

    isPlaying: false,
    isRecording: false,
    loopEnabled: false,
    loopStart: 0,
    loopEnd: 16,
    currentBeat: 0,
    metronomeEnabled: true,
    countIn: 0,
    preRoll: false,
    followPlayhead: true,

    tracks: initialTracks,
    masterTrack,
    returnTracks,
    selectedTrackId: initialTracks[0].id,

    scenes,
    selectedSceneIndex: 0,

    activeView: 'session',
    showBrowser: true,
    showDetail: true,
    showMixer: true,
    detailView: 'device',
    selectedClipId: null,
    horizontalZoom: 1,
    verticalZoom: 1,
    scrollX: 0,
    scrollY: 0,
    drawMode: false,
    snapToGrid: true,
    gridSize: 0.25,
    soloExclusive: true,

    browserCategory: 'sounds',
    browserSearchQuery: '',

    clipboardClip: null,
    clipboardNotes: null,

    undoStack: [],
    redoStack: [],

    // Project
    setProjectName: (name) => set({ projectName: name }),
    setBPM: (bpm) => set({ bpm: Math.max(20, Math.min(999, bpm)) }),
    setTimeSignature: (ts) => set({ timeSignature: ts }),

    // Transport
    play: () => set({ isPlaying: true }),
    stop: () => set({ isPlaying: false, isRecording: false, currentBeat: 0 }),
    togglePlay: () => set((s) => ({ isPlaying: !s.isPlaying, isRecording: s.isPlaying ? false : s.isRecording })),
    record: () => set((s) => ({ isRecording: !s.isRecording, isPlaying: true })),
    setCurrentBeat: (beat) => set({ currentBeat: beat }),
    toggleLoop: () => set((s) => ({ loopEnabled: !s.loopEnabled })),
    setLoopRegion: (start, end) => set({ loopStart: start, loopEnd: end }),
    toggleMetronome: () => set((s) => ({ metronomeEnabled: !s.metronomeEnabled })),
    toggleFollowPlayhead: () => set((s) => ({ followPlayhead: !s.followPlayhead })),

    // Track actions
    addTrack: (type) => set((s) => ({
      tracks: [...s.tracks, createDefaultTrack(type, s.tracks.length)],
    })),
    removeTrack: (id) => set((s) => ({
      tracks: s.tracks.filter((t) => t.id !== id),
      selectedTrackId: s.selectedTrackId === id ? null : s.selectedTrackId,
    })),
    duplicateTrack: (id) => set((s) => {
      const track = s.tracks.find((t) => t.id === id);
      if (!track) return s;
      const newTrack = { ...track, id: uuid(), name: `${track.name} Copy` };
      const idx = s.tracks.findIndex((t) => t.id === id);
      const newTracks = [...s.tracks];
      newTracks.splice(idx + 1, 0, newTrack);
      return { tracks: newTracks };
    }),
    selectTrack: (id) => set({ selectedTrackId: id }),
    setTrackVolume: (id, volume) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, volume } : t),
      masterTrack: s.masterTrack.id === id ? { ...s.masterTrack, volume } : s.masterTrack,
      returnTracks: s.returnTracks.map((t) => t.id === id ? { ...t, volume } : t),
    })),
    setTrackPan: (id, pan) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, pan } : t),
    })),
    toggleTrackMute: (id) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, mute: !t.mute } : t),
    })),
    toggleTrackSolo: (id) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, solo: !t.solo } : t),
    })),
    toggleTrackArm: (id) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, armed: !t.armed } : t),
    })),
    setTrackColor: (id, color) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, color } : t),
    })),
    renameTrack: (id, name) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, name } : t),
    })),
    reorderTracks: (fromIndex, toIndex) => set((s) => {
      const newTracks = [...s.tracks];
      const [moved] = newTracks.splice(fromIndex, 1);
      newTracks.splice(toIndex, 0, moved);
      return { tracks: newTracks };
    }),
    setTrackMeter: (id, left, right) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === id ? { ...t, meterLeft: left, meterRight: right } : t),
      returnTracks: s.returnTracks.map((t) => t.id === id ? { ...t, meterLeft: left, meterRight: right } : t),
    })),
    setMasterMeter: (left, right) => set((s) => ({
      masterTrack: { ...s.masterTrack, meterLeft: left, meterRight: right },
    })),
    addDeviceToTrack: (trackId, device) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId ? { ...t, devices: [...t.devices, device] } : t),
    })),
    removeDeviceFromTrack: (trackId, deviceId) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? { ...t, devices: t.devices.filter((d) => d.id !== deviceId) }
        : t),
    })),
    setDeviceParameter: (trackId, deviceId, paramId, value) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? {
          ...t,
          devices: t.devices.map((d) => d.id === deviceId
            ? {
              ...d,
              parameters: d.parameters.map((p) =>
                p.id === paramId ? { ...p, value: Math.max(p.min, Math.min(p.max, value)) } : p
              ),
            }
            : d),
        }
        : t),
    })),
    toggleDeviceEnabled: (trackId, deviceId) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? { ...t, devices: t.devices.map((d) => d.id === deviceId ? { ...d, enabled: !d.enabled } : d) }
        : t),
    })),

    // Clip actions
    createClip: (trackId, sceneIndex) => set((s) => {
      const track = s.tracks.find((t) => t.id === trackId);
      if (!track) return s;
      const clip = createDefaultClip(trackId, track.type === 'audio' ? 'audio' : 'midi', sceneIndex, track.color);
      return {
        tracks: s.tracks.map((t) => t.id === trackId
          ? { ...t, clips: { ...t.clips, [sceneIndex]: clip } }
          : t),
        selectedClipId: clip.id,
      };
    }),
    deleteClip: (trackId, sceneIndex) => set((s) => ({
      tracks: s.tracks.map((t) => {
        if (t.id !== trackId) return t;
        const newClips = { ...t.clips };
        delete newClips[sceneIndex];
        return { ...t, clips: newClips };
      }),
    })),
    setClipPlaying: (trackId, clipId, playing) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? {
          ...t,
          clips: Object.fromEntries(
            Object.entries(t.clips).map(([si, c]) => [si, { ...c, isPlaying: c.id === clipId ? playing : false }])
          ),
          activeClipId: playing ? clipId : null,
        }
        : t),
    })),
    launchClip: (trackId, sceneIndex) => {
      const state = get();
      const track = state.tracks.find((t) => t.id === trackId);
      if (!track || !track.clips[sceneIndex]) return;
      set((s) => ({
        tracks: s.tracks.map((t) => t.id === trackId
          ? {
            ...t,
            clips: Object.fromEntries(
              Object.entries(t.clips).map(([si, c]) => [
                si,
                { ...c, isPlaying: Number(si) === sceneIndex },
              ])
            ),
            activeClipId: t.clips[sceneIndex]?.id ?? null,
          }
          : t),
      }));
    },
    stopTrackClip: (trackId) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? {
          ...t,
          clips: Object.fromEntries(
            Object.entries(t.clips).map(([si, c]) => [si, { ...c, isPlaying: false }])
          ),
          activeClipId: null,
        }
        : t),
    })),
    launchScene: (sceneIndex) => {
      const state = get();
      state.tracks.forEach((track) => {
        if (track.clips[sceneIndex]) {
          state.launchClip(track.id, sceneIndex);
        }
      });
    },
    stopAllClips: () => set((s) => ({
      tracks: s.tracks.map((t) => ({
        ...t,
        clips: Object.fromEntries(
          Object.entries(t.clips).map(([si, c]) => [si, { ...c, isPlaying: false }])
        ),
        activeClipId: null,
      })),
    })),
    selectClip: (clipId) => set({ selectedClipId: clipId, detailView: 'clip' }),
    updateClip: (clipId, updates) => set((s) => ({
      tracks: s.tracks.map((t) => ({
        ...t,
        clips: Object.fromEntries(
          Object.entries(t.clips).map(([si, c]) =>
            [si, c.id === clipId ? { ...c, ...updates } : c]
          )
        ),
        arrangementClips: t.arrangementClips.map((c) =>
          c.id === clipId ? { ...c, ...updates } : c
        ),
      })),
    })),

    // Arrangement clip actions
    addArrangementClip: (trackId, clip) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? { ...t, arrangementClips: [...t.arrangementClips, clip] }
        : t),
    })),
    moveArrangementClip: (trackId, clipId, newStart) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? { ...t, arrangementClips: t.arrangementClips.map((c) => c.id === clipId ? { ...c, startTime: newStart } : c) }
        : t),
    })),
    resizeArrangementClip: (trackId, clipId, newDuration) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? { ...t, arrangementClips: t.arrangementClips.map((c) => c.id === clipId ? { ...c, duration: newDuration } : c) }
        : t),
    })),
    splitArrangementClip: (trackId, clipId, splitPoint) => set((s) => ({
      tracks: s.tracks.map((t) => {
        if (t.id !== trackId) return t;
        const clipIdx = t.arrangementClips.findIndex((c) => c.id === clipId);
        if (clipIdx < 0) return t;
        const clip = t.arrangementClips[clipIdx];
        const relSplit = splitPoint - clip.startTime;
        if (relSplit <= 0 || relSplit >= clip.duration) return t;
        const clip1: Clip = { ...clip, duration: relSplit };
        const clip2: Clip = {
          ...clip,
          id: uuid(),
          name: clip.name + ' (2)',
          startTime: splitPoint,
          duration: clip.duration - relSplit,
        };
        const newClips = [...t.arrangementClips];
        newClips.splice(clipIdx, 1, clip1, clip2);
        return { ...t, arrangementClips: newClips };
      }),
    })),
    deleteArrangementClip: (trackId, clipId) => set((s) => ({
      tracks: s.tracks.map((t) => t.id === trackId
        ? { ...t, arrangementClips: t.arrangementClips.filter((c) => c.id !== clipId) }
        : t),
    })),

    // MIDI actions
    addNote: (clipId, note) => set((s) => ({
      tracks: s.tracks.map((t) => ({
        ...t,
        clips: Object.fromEntries(
          Object.entries(t.clips).map(([si, c]) => [
            si,
            c.id === clipId && c.notes
              ? { ...c, notes: [...c.notes, note] }
              : c,
          ])
        ),
        arrangementClips: t.arrangementClips.map((c) =>
          c.id === clipId && c.notes ? { ...c, notes: [...c.notes, note] } : c
        ),
      })),
    })),
    removeNote: (clipId, noteId) => set((s) => ({
      tracks: s.tracks.map((t) => ({
        ...t,
        clips: Object.fromEntries(
          Object.entries(t.clips).map(([si, c]) => [
            si,
            c.id === clipId && c.notes
              ? { ...c, notes: c.notes.filter((n) => n.id !== noteId) }
              : c,
          ])
        ),
        arrangementClips: t.arrangementClips.map((c) =>
          c.id === clipId && c.notes
            ? { ...c, notes: c.notes.filter((n) => n.id !== noteId) }
            : c
        ),
      })),
    })),
    updateNote: (clipId, noteId, updates) => set((s) => ({
      tracks: s.tracks.map((t) => ({
        ...t,
        clips: Object.fromEntries(
          Object.entries(t.clips).map(([si, c]) => [
            si,
            c.id === clipId && c.notes
              ? { ...c, notes: c.notes.map((n) => n.id === noteId ? { ...n, ...updates } : n) }
              : c,
          ])
        ),
        arrangementClips: t.arrangementClips.map((c) =>
          c.id === clipId && c.notes
            ? { ...c, notes: c.notes.map((n) => n.id === noteId ? { ...n, ...updates } : n) }
            : c
        ),
      })),
    })),
    moveNote: (clipId, noteId, pitch, startTime) => {
      get().updateNote(clipId, noteId, { pitch, startTime });
    },
    resizeNote: (clipId, noteId, duration) => {
      get().updateNote(clipId, noteId, { duration: Math.max(0.0625, duration) });
    },

    // Scene actions
    addScene: () => set((s) => ({
      scenes: [...s.scenes, createDefaultScene(s.scenes.length)],
    })),
    removeScene: (index) => set((s) => ({
      scenes: s.scenes.filter((_, i) => i !== index),
    })),
    renameScene: (index, name) => set((s) => ({
      scenes: s.scenes.map((sc, i) => i === index ? { ...sc, name } : sc),
    })),
    selectScene: (index) => set({ selectedSceneIndex: index }),

    // View actions
    toggleView: () => set((s) => ({
      activeView: s.activeView === 'session' ? 'arrangement' : 'session',
    })),
    setActiveView: (view) => set({ activeView: view }),
    toggleBrowser: () => set((s) => ({ showBrowser: !s.showBrowser })),
    toggleDetail: () => set((s) => ({ showDetail: !s.showDetail })),
    toggleMixer: () => set((s) => ({ showMixer: !s.showMixer })),
    setDetailView: (view) => set({ detailView: view }),
    setHorizontalZoom: (zoom) => set({ horizontalZoom: Math.max(0.1, Math.min(10, zoom)) }),
    setVerticalZoom: (zoom) => set({ verticalZoom: Math.max(0.5, Math.min(4, zoom)) }),
    setScrollX: (x) => set({ scrollX: Math.max(0, x) }),
    setScrollY: (y) => set({ scrollY: Math.max(0, y) }),
    toggleDrawMode: () => set((s) => ({ drawMode: !s.drawMode })),
    toggleSnapToGrid: () => set((s) => ({ snapToGrid: !s.snapToGrid })),
    setGridSize: (size) => set({ gridSize: size }),
    setBrowserCategory: (cat) => set({ browserCategory: cat }),
    setBrowserSearch: (query) => set({ browserSearchQuery: query }),
  };
});
