import * as Tone from 'tone';
import { v4 as uuid } from 'uuid';
import type { Clip, MidiNote, Device, DeviceParameter } from '../types';

interface TrackChannel {
  id: string;
  channel: Tone.Channel;
  meter: Tone.Meter;
  instrument?: Tone.PolySynth | Tone.Synth | Tone.FMSynth | Tone.AMSynth | Tone.MonoSynth | Tone.MetalSynth | Tone.MembraneSynth | Tone.PluckSynth | Tone.NoiseSynth;
  effects: Map<string, Tone.ToneAudioNode>;
  player?: Tone.Player;
  players: Map<string, Tone.Player>;
  scheduledEvents: number[];
  recorder?: MediaRecorder;
  analyser?: Tone.Analyser;
}

export type SynthType = 'poly' | 'fm' | 'am' | 'mono' | 'membrane' | 'metal' | 'pluck' | 'noise' | 'duo' | 'wavetable';
export type EffectType = 'reverb' | 'delay' | 'chorus' | 'phaser' | 'distortion' | 'compressor' | 'eq3' | 'filter' | 'tremolo' | 'vibrato' | 'bitcrusher' | 'chebyshev' | 'freeverb' | 'pingpong' | 'autowah' | 'pitchshift' | 'feedbackdelay';

const SYNTH_PRESETS: Record<string, () => Tone.PolySynth | Tone.Synth | any> = {
  'poly': () => new Tone.PolySynth(Tone.Synth, {
    oscillator: { type: 'sawtooth' },
    envelope: { attack: 0.01, decay: 0.3, sustain: 0.5, release: 0.8 },
  }),
  'fm': () => new Tone.PolySynth(Tone.FMSynth, {
    harmonicity: 3,
    modulationIndex: 10,
    envelope: { attack: 0.01, decay: 0.2, sustain: 0.5, release: 0.5 },
  }),
  'am': () => new Tone.PolySynth(Tone.AMSynth, {
    harmonicity: 2,
    envelope: { attack: 0.01, decay: 0.3, sustain: 0.6, release: 0.8 },
  }),
  'mono': () => new Tone.MonoSynth({
    oscillator: { type: 'square' },
    filter: { Q: 6, type: 'lowpass', rolloff: -24 },
    envelope: { attack: 0.005, decay: 0.1, sustain: 0.9, release: 1 },
    filterEnvelope: { attack: 0.06, decay: 0.2, sustain: 0.5, release: 2, baseFrequency: 200, octaves: 7, exponent: 2 },
  }),
  'membrane': () => new Tone.MembraneSynth({
    pitchDecay: 0.05,
    octaves: 10,
    oscillator: { type: 'sine' },
    envelope: { attack: 0.001, decay: 0.4, sustain: 0.01, release: 1.4 },
  }),
  'metal': () => {
    const s = new Tone.MetalSynth({
      envelope: { attack: 0.001, decay: 1.4, release: 0.2 },
      harmonicity: 5.1,
      modulationIndex: 32,
      resonance: 4000,
      octaves: 1.5,
    });
    s.frequency.value = 200;
    return s;
  },
  'pluck': () => new Tone.PluckSynth({
    attackNoise: 1,
    dampening: 4000,
    resonance: 0.98,
  }),
  'noise': () => new Tone.NoiseSynth({
    noise: { type: 'white' },
    envelope: { attack: 0.005, decay: 0.1, sustain: 0 },
  }),
  'duo': () => new Tone.PolySynth(Tone.Synth, {
    oscillator: { type: 'fatsawtooth', count: 3, spread: 30 },
    envelope: { attack: 0.01, decay: 0.1, sustain: 0.5, release: 0.4 },
  }),
  'wavetable': () => new Tone.PolySynth(Tone.Synth, {
    oscillator: { type: 'triangle8' },
    envelope: { attack: 0.05, decay: 0.5, sustain: 0.7, release: 1.2 },
  }),
};

function createEffect(type: EffectType, params?: Record<string, number>): Tone.ToneAudioNode {
  switch (type) {
    case 'reverb': return new Tone.Reverb({ decay: params?.decay ?? 2.5, wet: params?.wet ?? 0.3 });
    case 'delay': return new Tone.FeedbackDelay({ delayTime: params?.time ?? '8n', feedback: params?.feedback ?? 0.3, wet: params?.wet ?? 0.3 });
    case 'chorus': return new Tone.Chorus({ frequency: params?.frequency ?? 1.5, delayTime: params?.delay ?? 3.5, depth: params?.depth ?? 0.7, wet: params?.wet ?? 0.3 });
    case 'phaser': return new Tone.Phaser({ frequency: params?.frequency ?? 0.5, octaves: params?.octaves ?? 3, baseFrequency: params?.base ?? 1000, wet: params?.wet ?? 0.3 });
    case 'distortion': return new Tone.Distortion({ distortion: params?.amount ?? 0.4, wet: params?.wet ?? 0.5 });
    case 'compressor': return new Tone.Compressor({ threshold: params?.threshold ?? -24, ratio: params?.ratio ?? 4, attack: params?.attack ?? 0.003, release: params?.release ?? 0.25 });
    case 'eq3': return new Tone.EQ3({ low: params?.low ?? 0, mid: params?.mid ?? 0, high: params?.high ?? 0 });
    case 'filter': return new Tone.Filter({ frequency: params?.frequency ?? 1000, type: 'lowpass', rolloff: -12 });
    case 'tremolo': return new Tone.Tremolo({ frequency: params?.frequency ?? 4, depth: params?.depth ?? 1, wet: params?.wet ?? 0.5 }).start();
    case 'vibrato': return new Tone.Vibrato({ frequency: params?.frequency ?? 5, depth: params?.depth ?? 0.1, wet: params?.wet ?? 0.3 });
    case 'bitcrusher': return new Tone.BitCrusher({ bits: params?.bits ?? 4 });
    case 'chebyshev': return new Tone.Chebyshev(params?.order ?? 50);
    case 'freeverb': return new Tone.Freeverb({ roomSize: params?.room ?? 0.7, dampening: params?.damp ?? 3000, wet: params?.wet ?? 0.3 });
    case 'pingpong': return new Tone.PingPongDelay({ delayTime: params?.time ?? '4n', feedback: params?.feedback ?? 0.4, wet: params?.wet ?? 0.3 });
    case 'autowah': return new Tone.AutoWah({ baseFrequency: params?.base ?? 100, octaves: params?.octaves ?? 6, sensitivity: params?.sensitivity ?? 0, wet: params?.wet ?? 0.5 });
    case 'pitchshift': return new Tone.PitchShift({ pitch: params?.pitch ?? 0, wet: params?.wet ?? 1 });
    case 'feedbackdelay': return new Tone.FeedbackDelay({ delayTime: params?.time ?? 0.25, feedback: params?.feedback ?? 0.5, wet: params?.wet ?? 0.3 });
    default: return new Tone.Gain(1);
  }
}

export function getEffectParameters(type: EffectType): DeviceParameter[] {
  const p = (id: string, name: string, value: number, min: number, max: number, step = 0.01, unit = ''): DeviceParameter =>
    ({ id, name, value, min, max, step, unit, defaultValue: value });
  switch (type) {
    case 'reverb': return [p('decay', 'Decay', 2.5, 0.1, 30, 0.1, 's'), p('wet', 'Dry/Wet', 0.3, 0, 1)];
    case 'delay': return [p('time', 'Time', 0.25, 0.01, 2, 0.01, 's'), p('feedback', 'Feedback', 0.3, 0, 0.95), p('wet', 'Dry/Wet', 0.3, 0, 1)];
    case 'chorus': return [p('frequency', 'Rate', 1.5, 0.1, 10, 0.1, 'Hz'), p('depth', 'Depth', 0.7, 0, 1), p('wet', 'Dry/Wet', 0.3, 0, 1)];
    case 'phaser': return [p('frequency', 'Rate', 0.5, 0.1, 20, 0.1, 'Hz'), p('octaves', 'Octaves', 3, 1, 6, 1), p('wet', 'Dry/Wet', 0.3, 0, 1)];
    case 'distortion': return [p('amount', 'Drive', 0.4, 0, 1), p('wet', 'Dry/Wet', 0.5, 0, 1)];
    case 'compressor': return [p('threshold', 'Threshold', -24, -60, 0, 1, 'dB'), p('ratio', 'Ratio', 4, 1, 20, 0.5), p('attack', 'Attack', 0.003, 0.001, 1, 0.001, 's'), p('release', 'Release', 0.25, 0.01, 2, 0.01, 's')];
    case 'eq3': return [p('low', 'Low', 0, -24, 24, 0.5, 'dB'), p('mid', 'Mid', 0, -24, 24, 0.5, 'dB'), p('high', 'High', 0, -24, 24, 0.5, 'dB')];
    case 'filter': return [p('frequency', 'Frequency', 1000, 20, 20000, 1, 'Hz'), p('Q', 'Resonance', 1, 0.1, 18, 0.1)];
    case 'tremolo': return [p('frequency', 'Rate', 4, 0.1, 20, 0.1, 'Hz'), p('depth', 'Depth', 1, 0, 1), p('wet', 'Dry/Wet', 0.5, 0, 1)];
    case 'vibrato': return [p('frequency', 'Rate', 5, 0.1, 30, 0.1, 'Hz'), p('depth', 'Depth', 0.1, 0, 1), p('wet', 'Dry/Wet', 0.3, 0, 1)];
    case 'bitcrusher': return [p('bits', 'Bits', 4, 1, 16, 1, 'bits')];
    case 'freeverb': return [p('room', 'Room Size', 0.7, 0, 1), p('damp', 'Dampening', 3000, 100, 10000, 100, 'Hz'), p('wet', 'Dry/Wet', 0.3, 0, 1)];
    case 'pingpong': return [p('feedback', 'Feedback', 0.4, 0, 0.95), p('wet', 'Dry/Wet', 0.3, 0, 1)];
    case 'autowah': return [p('base', 'Base Freq', 100, 20, 2000, 1, 'Hz'), p('octaves', 'Octaves', 6, 1, 8, 1), p('wet', 'Dry/Wet', 0.5, 0, 1)];
    case 'pitchshift': return [p('pitch', 'Pitch', 0, -24, 24, 1, 'st'), p('wet', 'Dry/Wet', 1, 0, 1)];
    default: return [p('wet', 'Dry/Wet', 0.5, 0, 1)];
  }
}

export function getSynthParameters(type: SynthType): DeviceParameter[] {
  const p = (id: string, name: string, value: number, min: number, max: number, step = 0.01, unit = ''): DeviceParameter =>
    ({ id, name, value, min, max, step, unit, defaultValue: value });
  const envParams = [
    p('attack', 'Attack', 0.01, 0.001, 2, 0.001, 's'),
    p('decay', 'Decay', 0.3, 0.01, 5, 0.01, 's'),
    p('sustain', 'Sustain', 0.5, 0, 1),
    p('release', 'Release', 0.8, 0.01, 10, 0.01, 's'),
  ];
  switch (type) {
    case 'poly': return [...envParams, p('detune', 'Detune', 0, -1200, 1200, 1, 'ct')];
    case 'fm': return [p('harmonicity', 'Harmonicity', 3, 0.5, 20, 0.1), p('modulationIndex', 'Mod Index', 10, 0.1, 100, 0.1), ...envParams];
    case 'am': return [p('harmonicity', 'Harmonicity', 2, 0.5, 20, 0.1), ...envParams];
    case 'mono': return [...envParams, p('filterFreq', 'Filter Freq', 2000, 20, 20000, 1, 'Hz'), p('filterQ', 'Filter Q', 6, 0.1, 18, 0.1)];
    case 'membrane': return [p('pitchDecay', 'Pitch Decay', 0.05, 0, 0.5, 0.01), p('octaves', 'Octaves', 10, 0.5, 16, 0.5), ...envParams];
    case 'metal': return [p('frequency', 'Frequency', 200, 20, 2000, 1, 'Hz'), p('harmonicity', 'Harmonicity', 5.1, 0.1, 20, 0.1), p('modulationIndex', 'Mod Index', 32, 1, 100, 1), p('resonance', 'Resonance', 4000, 100, 10000, 100, 'Hz')];
    case 'pluck': return [p('attackNoise', 'Attack Noise', 1, 0.1, 20, 0.1), p('dampening', 'Dampening', 4000, 100, 10000, 100, 'Hz'), p('resonance', 'Resonance', 0.98, 0.1, 0.999, 0.001)];
    case 'noise': return [p('attack', 'Attack', 0.005, 0.001, 2, 0.001, 's'), p('decay', 'Decay', 0.1, 0.01, 5, 0.01, 's'), p('sustain', 'Sustain', 0, 0, 1)];
    case 'duo': return [...envParams, p('spread', 'Spread', 30, 0, 100, 1, 'ct'), p('count', 'Voices', 3, 1, 8, 1)];
    case 'wavetable': return [...envParams, p('partials', 'Partials', 8, 1, 32, 1)];
    default: return envParams;
  }
}

export class AudioEngine {
  private trackChannels: Map<string, TrackChannel> = new Map();
  private masterChannel!: Tone.Channel;
  private masterMeter!: Tone.Meter;
  private masterLimiter!: Tone.Limiter;
  private metronome!: Tone.Synth;
  private metronomePart!: Tone.Loop;
  private returnChannels: Map<string, TrackChannel> = new Map();
  private isInitialized = false;
  private meterCallback?: (trackId: string, left: number, right: number) => void;
  private masterMeterCallback?: (left: number, right: number) => void;
  private beatCallback?: (beat: number) => void;
  private meterInterval?: ReturnType<typeof setInterval>;

  async init(): Promise<void> {
    if (this.isInitialized) return;
    await Tone.start();

    this.masterLimiter = new Tone.Limiter(-1).toDestination();
    this.masterChannel = new Tone.Channel({ volume: 0 }).connect(this.masterLimiter);
    this.masterMeter = new Tone.Meter({ smoothing: 0.8 });
    this.masterChannel.connect(this.masterMeter);

    // Metronome
    this.metronome = new Tone.Synth({
      oscillator: { type: 'triangle' },
      envelope: { attack: 0.001, decay: 0.1, sustain: 0, release: 0.05 },
      volume: -10,
    }).connect(this.masterChannel);

    this.metronomePart = new Tone.Loop((time) => {
      const beat = Math.floor(Tone.Transport.position as unknown as number) % 4;
      this.metronome.triggerAttackRelease(beat === 0 ? 'C6' : 'C5', '32n', time);
    }, '4n');

    this.isInitialized = true;
    this.startMetering();
  }

  dispose(): void {
    if (this.meterInterval) clearInterval(this.meterInterval);
    Tone.Transport.stop();
    Tone.Transport.cancel();
    this.trackChannels.forEach((tc) => {
      tc.channel.dispose();
      tc.meter.dispose();
      tc.instrument?.dispose();
      tc.effects.forEach((e) => e.dispose());
      tc.players.forEach((p) => p.dispose());
    });
    this.trackChannels.clear();
    this.masterChannel?.dispose();
    this.masterMeter?.dispose();
    this.masterLimiter?.dispose();
    this.metronome?.dispose();
    this.metronomePart?.dispose();
    this.isInitialized = false;
  }

  // Transport
  play(): void {
    Tone.Transport.start();
  }

  stop(): void {
    Tone.Transport.stop();
    Tone.Transport.position = 0;
    this.trackChannels.forEach((tc) => {
      tc.scheduledEvents.forEach((id) => Tone.Transport.clear(id));
      tc.scheduledEvents = [];
    });
  }

  pause(): void {
    Tone.Transport.pause();
  }

  get isPlaying(): boolean {
    return Tone.Transport.state === 'started';
  }

  setTempo(bpm: number): void {
    Tone.Transport.bpm.value = bpm;
  }

  setLoop(enabled: boolean, start: number, end: number): void {
    Tone.Transport.loop = enabled;
    if (enabled) {
      Tone.Transport.loopStart = `${start}:0:0`;
      Tone.Transport.loopEnd = `${end}:0:0`;
    }
  }

  setPosition(bars: number): void {
    Tone.Transport.position = `${Math.floor(bars)}:0:0`;
  }

  getPosition(): number {
    const pos = Tone.Transport.position;
    if (typeof pos === 'number') return pos;
    const parts = String(pos).split(':').map(Number);
    return parts[0] + (parts[1] || 0) / 4 + (parts[2] || 0) / 16;
  }

  getPositionBeats(): number {
    return Tone.Transport.seconds * (Tone.Transport.bpm.value / 60);
  }

  setMetronome(enabled: boolean): void {
    if (enabled) {
      this.metronomePart.start(0);
    } else {
      this.metronomePart.stop();
    }
  }

  setTimeSignature(numerator: number, denominator: number): void {
    Tone.Transport.timeSignature = [numerator, denominator];
  }

  // Metering
  onMeter(callback: (trackId: string, left: number, right: number) => void): void {
    this.meterCallback = callback;
  }

  onMasterMeter(callback: (left: number, right: number) => void): void {
    this.masterMeterCallback = callback;
  }

  onBeat(callback: (beat: number) => void): void {
    this.beatCallback = callback;
  }

  private startMetering(): void {
    this.meterInterval = setInterval(() => {
      // Track meters
      this.trackChannels.forEach((tc, id) => {
        const level = tc.meter.getValue();
        let left: number, right: number;
        if (Array.isArray(level)) {
          left = Math.max(0, Math.min(1, (level[0] as number + 60) / 60));
          right = Math.max(0, Math.min(1, ((level[1] as number ?? level[0] as number) + 60) / 60));
        } else {
          const norm = Math.max(0, Math.min(1, ((level as number) + 60) / 60));
          left = right = norm;
        }
        this.meterCallback?.(id, left, right);
      });

      // Return track meters
      this.returnChannels.forEach((tc, id) => {
        const level = tc.meter.getValue();
        let left: number, right: number;
        if (Array.isArray(level)) {
          left = Math.max(0, Math.min(1, (level[0] as number + 60) / 60));
          right = Math.max(0, Math.min(1, ((level[1] as number ?? level[0] as number) + 60) / 60));
        } else {
          const norm = Math.max(0, Math.min(1, ((level as number) + 60) / 60));
          left = right = norm;
        }
        this.meterCallback?.(id, left, right);
      });

      // Master meter
      if (this.masterMeter) {
        const ml = this.masterMeter.getValue();
        let mLeft: number, mRight: number;
        if (Array.isArray(ml)) {
          mLeft = Math.max(0, Math.min(1, (ml[0] as number + 60) / 60));
          mRight = Math.max(0, Math.min(1, ((ml[1] as number ?? ml[0] as number) + 60) / 60));
        } else {
          const norm = Math.max(0, Math.min(1, ((ml as number) + 60) / 60));
          mLeft = mRight = norm;
        }
        this.masterMeterCallback?.(mLeft, mRight);
      }

      // Beat position
      if (Tone.Transport.state === 'started') {
        this.beatCallback?.(this.getPosition());
      }
    }, 50);
  }

  // Track management
  createTrack(id: string, type: 'audio' | 'midi' | 'return' | 'master' | 'group'): void {
    if (this.trackChannels.has(id) || this.returnChannels.has(id)) return;

    const channel = new Tone.Channel({ volume: -6 });
    const meter = new Tone.Meter({ smoothing: 0.8 });
    channel.connect(meter);

    if (type === 'return') {
      channel.connect(this.masterChannel);
      this.returnChannels.set(id, {
        id,
        channel,
        meter,
        effects: new Map(),
        players: new Map(),
        scheduledEvents: [],
      });
    } else {
      channel.connect(this.masterChannel);
      const tc: TrackChannel = {
        id,
        channel,
        meter,
        effects: new Map(),
        players: new Map(),
        scheduledEvents: [],
      };
      if (type === 'midi') {
        tc.instrument = SYNTH_PRESETS['poly']();
        tc.instrument!.connect(channel);
      }
      this.trackChannels.set(id, tc);
    }
  }

  removeTrack(id: string): void {
    const tc = this.trackChannels.get(id) || this.returnChannels.get(id);
    if (!tc) return;
    tc.scheduledEvents.forEach((evId) => Tone.Transport.clear(evId));
    tc.instrument?.dispose();
    tc.effects.forEach((e) => e.dispose());
    tc.players.forEach((p) => p.dispose());
    tc.channel.dispose();
    tc.meter.dispose();
    this.trackChannels.delete(id);
    this.returnChannels.delete(id);
  }

  setTrackVolume(id: string, volume: number): void {
    const tc = this.trackChannels.get(id) || this.returnChannels.get(id);
    if (!tc) return;
    // volume 0-1 mapped to dB
    const db = volume <= 0 ? -Infinity : 20 * Math.log10(volume) * 1.5 + 6;
    tc.channel.volume.value = db;
  }

  setMasterVolume(volume: number): void {
    const db = volume <= 0 ? -Infinity : 20 * Math.log10(volume) * 1.5 + 6;
    this.masterChannel.volume.value = db;
  }

  setTrackPan(id: string, pan: number): void {
    const tc = this.trackChannels.get(id);
    if (!tc) return;
    tc.channel.pan.value = pan;
  }

  setTrackMute(id: string, mute: boolean): void {
    const tc = this.trackChannels.get(id);
    if (!tc) return;
    tc.channel.mute = mute;
  }

  setTrackSolo(id: string, solo: boolean): void {
    const tc = this.trackChannels.get(id);
    if (!tc) return;
    tc.channel.solo = solo;
  }

  // Instruments
  setInstrument(trackId: string, type: SynthType): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return;

    // Disconnect old instrument
    if (tc.instrument) {
      tc.instrument.disconnect();
      tc.instrument.dispose();
    }

    // Create new instrument
    const preset = SYNTH_PRESETS[type];
    if (preset) {
      tc.instrument = preset();
      // Connect through effect chain or directly to channel
      if (tc.effects.size > 0) {
        const effectsArray = Array.from(tc.effects.values());
        tc.instrument!.connect(effectsArray[0]);
      } else {
        tc.instrument!.connect(tc.channel);
      }
    }
  }

  // Effects
  addEffect(trackId: string, type: EffectType, params?: Record<string, number>): string {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return '';

    const effectId = uuid();
    const effect = createEffect(type, params);

    // Rebuild effect chain
    tc.effects.set(effectId, effect);
    this.rebuildEffectChain(tc);

    return effectId;
  }

  removeEffect(trackId: string, effectId: string): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return;

    const effect = tc.effects.get(effectId);
    if (effect) {
      effect.disconnect();
      effect.dispose();
      tc.effects.delete(effectId);
      this.rebuildEffectChain(tc);
    }
  }

  setEffectParam(trackId: string, effectId: string, param: string, value: number): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return;
    const effect = tc.effects.get(effectId) as any;
    if (!effect) return;
    try {
      if (param in effect) {
        const prop = effect[param];
        if (prop && typeof prop === 'object' && 'value' in prop) {
          prop.value = value;
        } else {
          effect[param] = value;
        }
      }
    } catch (e) {
      // Some params may not be settable
    }
  }

  private rebuildEffectChain(tc: TrackChannel): void {
    // Disconnect everything
    tc.instrument?.disconnect();
    tc.effects.forEach((e) => e.disconnect());

    const effectsArray = Array.from(tc.effects.values());

    if (effectsArray.length === 0) {
      tc.instrument?.connect(tc.channel);
      return;
    }

    // Connect: instrument -> effect1 -> effect2 -> ... -> channel
    tc.instrument?.connect(effectsArray[0]);
    for (let i = 0; i < effectsArray.length - 1; i++) {
      effectsArray[i].connect(effectsArray[i + 1] as any);
    }
    effectsArray[effectsArray.length - 1].connect(tc.channel);
  }

  // MIDI Playback
  playNote(trackId: string, note: string | number, velocity: number = 0.8, duration: string | number = '8n'): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc?.instrument) return;

    const noteStr = typeof note === 'number' ? Tone.Frequency(note, 'midi').toNote() : note;
    try {
      if ('triggerAttackRelease' in tc.instrument) {
        (tc.instrument as any).triggerAttackRelease(noteStr, duration, undefined, velocity);
      }
    } catch (e) {
      // Some synths don't support all operations
    }
  }

  noteOn(trackId: string, note: number, velocity: number = 0.8): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc?.instrument) return;
    const noteStr = Tone.Frequency(note, 'midi').toNote();
    try {
      if ('triggerAttack' in tc.instrument) {
        (tc.instrument as any).triggerAttack(noteStr, undefined, velocity);
      }
    } catch (e) {}
  }

  noteOff(trackId: string, note: number): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc?.instrument) return;
    const noteStr = Tone.Frequency(note, 'midi').toNote();
    try {
      if ('triggerRelease' in tc.instrument) {
        (tc.instrument as any).triggerRelease(noteStr);
      }
    } catch (e) {}
  }

  // Schedule MIDI clip playback
  scheduleMidiClip(trackId: string, notes: MidiNote[], startBar: number, clipDuration: number, loop: boolean): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc?.instrument) return;

    // Clear existing events for this track
    tc.scheduledEvents.forEach((id) => Tone.Transport.clear(id));
    tc.scheduledEvents = [];

    const bpm = Tone.Transport.bpm.value;
    const beatsPerBar = 4; // assume 4/4

    notes.forEach((note) => {
      if (note.muted) return;
      const noteStr = Tone.Frequency(note.pitch, 'midi').toNote();
      const noteTimeInBeats = startBar * beatsPerBar + note.startTime;
      const noteDurationStr = `${note.duration * (60 / bpm)}`;

      const eventId = Tone.Transport.schedule((time) => {
        if (Math.random() <= note.probability) {
          try {
            (tc.instrument as any).triggerAttackRelease(
              noteStr,
              note.duration * (60 / bpm),
              time,
              note.velocity / 127
            );
          } catch (e) {}
        }
      }, `${noteTimeInBeats * (60 / bpm)}`);

      tc.scheduledEvents.push(eventId as unknown as number);
    });
  }

  // Audio file loading
  async loadAudioFile(file: File): Promise<{ url: string; duration: number }> {
    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = async (e) => {
        try {
          const arrayBuffer = e.target?.result as ArrayBuffer;
          const audioContext = Tone.getContext().rawContext;
          if (!audioContext) throw new Error('No audio context');
          const audioBuffer = await (audioContext as any).decodeAudioData(arrayBuffer);
          const url = URL.createObjectURL(file);
          resolve({ url, duration: audioBuffer.duration });
        } catch (err) {
          reject(err);
        }
      };
      reader.onerror = reject;
      reader.readAsArrayBuffer(file);
    });
  }

  // Play audio clip
  playAudioClip(trackId: string, url: string, startTime?: number): string {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return '';

    const playerId = uuid();
    const player = new Tone.Player({
      url,
      onload: () => {
        player.connect(tc.channel);
        if (Tone.Transport.state === 'started') {
          player.start();
        }
      },
    });

    tc.players.set(playerId, player);
    return playerId;
  }

  stopAudioClip(trackId: string, playerId: string): void {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return;
    const player = tc.players.get(playerId);
    if (player) {
      player.stop();
      player.dispose();
      tc.players.delete(playerId);
    }
  }

  // Export
  async exportAudio(duration: number): Promise<Blob> {
    const recorder = new Tone.Recorder();
    this.masterChannel.connect(recorder);
    recorder.start();

    // Play through the arrangement
    this.setPosition(0);
    this.play();

    await new Promise((resolve) => setTimeout(resolve, duration * 1000));

    this.stop();
    const blob = await recorder.stop();
    this.masterChannel.disconnect(recorder);
    recorder.dispose();
    return blob;
  }

  // Recording from microphone
  async startMicRecording(trackId: string): Promise<void> {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return;

    const mic = new Tone.UserMedia();
    await mic.open();
    const recorder = new Tone.Recorder();
    mic.connect(recorder);
    mic.connect(tc.channel);
    recorder.start();
    (tc as any)._mic = mic;
    (tc as any)._recorder = recorder;
  }

  async stopMicRecording(trackId: string): Promise<Blob | null> {
    const tc = this.trackChannels.get(trackId);
    if (!tc) return null;

    const recorder = (tc as any)._recorder as Tone.Recorder;
    const mic = (tc as any)._mic as Tone.UserMedia;
    if (!recorder || !mic) return null;

    const blob = await recorder.stop();
    mic.close();
    mic.dispose();
    recorder.dispose();
    delete (tc as any)._mic;
    delete (tc as any)._recorder;
    return blob;
  }
}

// Singleton
export const audioEngine = new AudioEngine();
