const TRACK_COLORS = ['#58a6ff', '#3fb950', '#f85149', '#d29922', '#bc8cff', '#f778ba', '#db6d28', '#79c0ff'];

class AudioEngine {
  constructor() {
    this.ctx = null;
    this.masterGain = null;
    this.trackNodes = new Map();
    this.activeSources = new Map();
    this.isPlaying = false;
    this.startTime = 0;
    this.startOffset = 0;
    this.mediaRecorder = null;
    this.recordedChunks = [];
  }

  init() {
    if (this.ctx) return;
    this.ctx = new (window.AudioContext || window.webkitAudioContext)();
    this.masterGain = this.ctx.createGain();
    this.masterGain.connect(this.ctx.destination);
  }

  ensureContext() {
    this.init();
    if (this.ctx.state === 'suspended') {
      this.ctx.resume();
    }
  }

  createTrackNodes(trackId) {
    this.ensureContext();
    const gain = this.ctx.createGain();
    const pan = this.ctx.createStereoPanner();
    const eq = {
      low: this.ctx.createBiquadFilter(),
      mid: this.ctx.createBiquadFilter(),
      high: this.ctx.createBiquadFilter(),
    };
    eq.low.type = 'lowshelf';
    eq.low.frequency.value = 320;
    eq.mid.type = 'peaking';
    eq.mid.frequency.value = 1000;
    eq.mid.Q.value = 0.5;
    eq.high.type = 'highshelf';
    eq.high.frequency.value = 3200;

    gain.connect(pan);
    pan.connect(eq.low);
    eq.low.connect(eq.mid);
    eq.mid.connect(eq.high);
    eq.high.connect(this.masterGain);

    this.trackNodes.set(trackId, { gain, pan, eq });
    return { gain, pan, eq };
  }

  removeTrackNodes(trackId) {
    this.stopTrack(trackId);
    const nodes = this.trackNodes.get(trackId);
    if (nodes) {
      nodes.gain.disconnect();
      nodes.pan.disconnect();
      nodes.eq.low.disconnect();
      nodes.eq.mid.disconnect();
      nodes.eq.high.disconnect();
      this.trackNodes.delete(trackId);
    }
  }

  setTrackVolume(trackId, value) {
    const nodes = this.trackNodes.get(trackId);
    if (nodes) nodes.gain.gain.value = value;
  }

  setTrackPan(trackId, value) {
    const nodes = this.trackNodes.get(trackId);
    if (nodes) nodes.pan.pan.value = value;
  }

  setTrackMute(trackId, muted) {
    const nodes = this.trackNodes.get(trackId);
    if (nodes) nodes.gain.gain.value = muted ? 0 : 1;
  }

  setTrackEQ(trackId, band, value) {
    const nodes = this.trackNodes.get(trackId);
    if (nodes && nodes.eq[band]) {
      nodes.eq[band].gain.value = value;
    }
  }

  async decodeAudioData(arrayBuffer) {
    this.ensureContext();
    return this.ctx.decodeAudioData(arrayBuffer);
  }

  async loadFile(file) {
    const arrayBuffer = await file.arrayBuffer();
    return this.decodeAudioData(arrayBuffer);
  }

  async loadUrl(url) {
    const response = await fetch(url);
    const arrayBuffer = await response.arrayBuffer();
    return this.decodeAudioData(arrayBuffer);
  }

  playTracks(tracks, fromPosition = 0) {
    this.ensureContext();
    this.stopAll();
    this.isPlaying = true;
    this.startTime = this.ctx.currentTime;
    this.startOffset = fromPosition;

    tracks.forEach(track => {
      if (track.muted) return;
      const nodes = this.trackNodes.get(track.id);
      if (!nodes) return;

      const sources = [];
      track.regions.forEach(region => {
        if (!region.buffer) return;
        const regionEnd = region.startTime + region.buffer.duration;
        if (regionEnd <= fromPosition) return;

        const source = this.ctx.createBufferSource();
        source.buffer = region.buffer;
        source.connect(nodes.gain);

        const offset = Math.max(0, fromPosition - region.startTime);
        const when = Math.max(0, region.startTime - fromPosition);

        source.start(this.ctx.currentTime + when, offset);
        sources.push(source);
      });

      this.activeSources.set(track.id, sources);
    });
  }

  stopAll() {
    this.isPlaying = false;
    this.activeSources.forEach(sources => {
      sources.forEach(s => {
        try { s.stop(); } catch (e) { /* already stopped */ }
      });
    });
    this.activeSources.clear();
  }

  stopTrack(trackId) {
    const sources = this.activeSources.get(trackId);
    if (sources) {
      sources.forEach(s => {
        try { s.stop(); } catch (e) { /* already stopped */ }
      });
      this.activeSources.delete(trackId);
    }
  }

  getCurrentTime() {
    if (!this.isPlaying || !this.ctx) return this.startOffset;
    return this.startOffset + (this.ctx.currentTime - this.startTime);
  }

  async startRecording() {
    this.ensureContext();
    const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
    this.recordedChunks = [];
    this.mediaRecorder = new MediaRecorder(stream);
    this.mediaRecorder.ondataavailable = (e) => {
      if (e.data.size > 0) this.recordedChunks.push(e.data);
    };

    return new Promise((resolve) => {
      this.mediaRecorder.onstop = async () => {
        stream.getTracks().forEach(t => t.stop());
        const blob = new Blob(this.recordedChunks, { type: 'audio/webm' });
        const arrayBuffer = await blob.arrayBuffer();
        const audioBuffer = await this.decodeAudioData(arrayBuffer);
        resolve(audioBuffer);
      };
      this.mediaRecorder.start();
    });
  }

  stopRecording() {
    if (this.mediaRecorder && this.mediaRecorder.state === 'recording') {
      this.mediaRecorder.stop();
    }
  }

  async mixDown(tracks, duration) {
    this.ensureContext();
    const sampleRate = this.ctx.sampleRate;
    const offlineCtx = new OfflineAudioContext(2, sampleRate * duration, sampleRate);

    tracks.forEach(track => {
      if (track.muted) return;
      const gain = offlineCtx.createGain();
      gain.gain.value = track.volume;
      const pan = offlineCtx.createStereoPanner();
      pan.pan.value = track.pan;
      gain.connect(pan);
      pan.connect(offlineCtx.destination);

      track.regions.forEach(region => {
        if (!region.buffer) return;
        const source = offlineCtx.createBufferSource();
        source.buffer = region.buffer;
        source.connect(gain);
        source.start(region.startTime);
      });
    });

    const renderedBuffer = await offlineCtx.startRendering();
    return renderedBuffer;
  }

  audioBufferToWav(buffer) {
    const numChannels = buffer.numberOfChannels;
    const sampleRate = buffer.sampleRate;
    const format = 1; // PCM
    const bitDepth = 16;
    const bytesPerSample = bitDepth / 8;
    const blockAlign = numChannels * bytesPerSample;
    const dataLength = buffer.length * blockAlign;
    const headerLength = 44;
    const totalLength = headerLength + dataLength;
    const arrayBuffer = new ArrayBuffer(totalLength);
    const view = new DataView(arrayBuffer);

    const writeString = (offset, string) => {
      for (let i = 0; i < string.length; i++) {
        view.setUint8(offset + i, string.charCodeAt(i));
      }
    };

    writeString(0, 'RIFF');
    view.setUint32(4, totalLength - 8, true);
    writeString(8, 'WAVE');
    writeString(12, 'fmt ');
    view.setUint32(16, 16, true);
    view.setUint16(20, format, true);
    view.setUint16(22, numChannels, true);
    view.setUint32(24, sampleRate, true);
    view.setUint32(28, sampleRate * blockAlign, true);
    view.setUint16(32, blockAlign, true);
    view.setUint16(34, bitDepth, true);
    writeString(36, 'data');
    view.setUint32(40, dataLength, true);

    const channels = [];
    for (let ch = 0; ch < numChannels; ch++) {
      channels.push(buffer.getChannelData(ch));
    }

    let offset = 44;
    for (let i = 0; i < buffer.length; i++) {
      for (let ch = 0; ch < numChannels; ch++) {
        const sample = Math.max(-1, Math.min(1, channels[ch][i]));
        view.setInt16(offset, sample < 0 ? sample * 0x8000 : sample * 0x7fff, true);
        offset += 2;
      }
    }

    return new Blob([arrayBuffer], { type: 'audio/wav' });
  }
}

export const TRACK_COLOR_PALETTE = TRACK_COLORS;
export default new AudioEngine();
