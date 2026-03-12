export type TrackType = 'audio' | 'midi' | 'return' | 'master' | 'group';
export type ViewMode = 'session' | 'arrangement';
export type DetailView = 'clip' | 'device';
export type ClipType = 'audio' | 'midi';
export type LaunchMode = 'trigger' | 'gate' | 'toggle' | 'repeat';
export type WarpMode = 'beats' | 'tones' | 'texture' | 'repitch' | 'complex' | 'complex-pro';

export interface MidiNote {
  id: string;
  pitch: number;       // 0-127
  velocity: number;    // 0-127
  startTime: number;   // beats
  duration: number;    // beats
  probability: number; // 0-1
  muted: boolean;
}

export interface WarpMarker {
  sampleTime: number;
  beatTime: number;
}

export interface AutomationPoint {
  time: number;
  value: number;
  curve: number;      // -1 to 1
}

export interface AutomationLane {
  id: string;
  parameterId: string;
  parameterName: string;
  points: AutomationPoint[];
  enabled: boolean;
  min: number;
  max: number;
}

export interface Clip {
  id: string;
  trackId: string;
  name: string;
  color: string;
  type: ClipType;
  startTime: number;     // bars (arrangement position)
  duration: number;       // bars
  loopEnabled: boolean;
  loopStart: number;      // beats
  loopEnd: number;        // beats
  launchMode: LaunchMode;
  // Audio
  audioUrl?: string;
  audioBuffer?: Float32Array[];
  warpEnabled?: boolean;
  warpMode?: WarpMode;
  warpMarkers?: WarpMarker[];
  gain?: number;          // dB
  transpose?: number;     // semitones
  detune?: number;        // cents
  reversed?: boolean;
  // MIDI
  notes?: MidiNote[];
  // State
  isPlaying?: boolean;
  isRecording?: boolean;
  // Clip automation
  automation?: AutomationLane[];
}

export interface DeviceParameter {
  id: string;
  name: string;
  value: number;
  min: number;
  max: number;
  defaultValue: number;
  step?: number;
  unit?: string;
  choices?: string[];
}

export type InstrumentType =
  | 'wavetable' | 'analog' | 'fm' | 'drift' | 'electric'
  | 'simpler' | 'sampler' | 'drumrack' | 'external';

export type AudioEffectType =
  | 'compressor' | 'glue-compressor' | 'multiband-dynamics' | 'gate' | 'limiter'
  | 'eq-eight' | 'eq-three' | 'auto-filter' | 'channel-eq'
  | 'delay' | 'echo' | 'reverb' | 'hybrid-reverb'
  | 'chorus' | 'phaser' | 'flanger' | 'frequency-shifter'
  | 'saturator' | 'overdrive' | 'erosion' | 'redux' | 'pedal' | 'amp' | 'cabinet'
  | 'utility' | 'tuner' | 'spectrum' | 'audio-effect-rack' | 'external-effect';

export type MidiEffectType =
  | 'arpeggiator' | 'chord' | 'note-length' | 'pitch' | 'random' | 'scale' | 'velocity';

export interface Device {
  id: string;
  name: string;
  type: 'instrument' | 'audioEffect' | 'midiEffect';
  subType: InstrumentType | AudioEffectType | MidiEffectType;
  enabled: boolean;
  collapsed: boolean;
  parameters: DeviceParameter[];
}

export interface Send {
  id: string;
  returnTrackId: string;
  amount: number;     // 0-1
  preFader: boolean;
}

export interface Track {
  id: string;
  name: string;
  type: TrackType;
  color: string;
  volume: number;      // 0-1 (mapped to dB)
  pan: number;         // -1 to 1
  mute: boolean;
  solo: boolean;
  armed: boolean;
  monitorInput: boolean;
  devices: Device[];
  sends: Send[];
  clips: { [sceneIndex: number]: Clip };       // Session clips
  arrangementClips: Clip[];                      // Arrangement clips
  automation: AutomationLane[];
  inputSource: string;
  outputTarget: string;
  height: number;       // px in arrangement
  collapsed: boolean;
  children: string[];   // group track children
  freeze: boolean;
  // Runtime state
  meterLeft: number;
  meterRight: number;
  activeClipId: string | null;
}

export interface Scene {
  id: string;
  name: string;
  color: string;
  tempo?: number;
  timeSignature?: [number, number];
}

export interface Project {
  name: string;
  bpm: number;
  timeSignature: [number, number];
  sampleRate: number;
  tracks: Track[];
  scenes: Scene[];
  masterTrack: Track;
  returnTracks: Track[];
}

export const TRACK_COLORS = [
  '#e74c3c', '#e67e22', '#f1c40f', '#2ecc71',
  '#1abc9c', '#3498db', '#9b59b6', '#e91e63',
  '#00bcd4', '#8bc34a', '#ff9800', '#795548',
  '#607d8b', '#ff5722', '#cddc39', '#673ab7',
];

export const NOTE_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];

export function noteToName(note: number): string {
  const octave = Math.floor(note / 12) - 1;
  return `${NOTE_NAMES[note % 12]}${octave}`;
}

export function nameToNote(name: string): number {
  const match = name.match(/^([A-G]#?)(-?\d+)$/);
  if (!match) return 60;
  const noteIndex = NOTE_NAMES.indexOf(match[1]);
  const octave = parseInt(match[2]);
  return (octave + 1) * 12 + noteIndex;
}

export function dbToGain(db: number): number {
  if (db <= -70) return 0;
  return Math.pow(10, db / 20);
}

export function gainToDb(gain: number): number {
  if (gain <= 0) return -Infinity;
  return 20 * Math.log10(gain);
}

export function volumeToDb(volume: number): number {
  // Maps 0-1 fader position to dB (logarithmic)
  if (volume <= 0) return -Infinity;
  if (volume >= 1) return 6;
  // Attempt a roughly perceptual mapping
  return 20 * Math.log10(volume) * 1.5 + 6;
}
