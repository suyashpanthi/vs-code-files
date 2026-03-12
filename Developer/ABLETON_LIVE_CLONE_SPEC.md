# Full-Stack Digital Audio Workstation (DAW) — Complete Specification

## Prompt / Development Blueprint

Build a professional-grade Digital Audio Workstation (DAW) desktop application modeled after Ableton Live. The application must be a fully functional music production environment capable of recording, editing, arranging, mixing, and performing live with audio and MIDI. It must match or exceed Ableton Live in functionality, workflow, and visual design.

---

## 1. TECH STACK & ARCHITECTURE

### 1.1 Desktop Application
- **Framework**: Electron or Tauri (Rust backend for performance) — recommend Tauri for audio performance
- **Frontend**: React 19 + TypeScript with a custom component library
- **Audio Engine**: Written in Rust or C++ via native modules (WebAudio API is NOT sufficient for professional DAW work)
- **Audio Backend**: Use PortAudio, JACK, or CoreAudio/WASAPI/ALSA directly for low-latency audio I/O
- **MIDI Engine**: Native MIDI I/O via RtMidi or platform-native APIs
- **State Management**: Zustand or custom observable store (must handle thousands of state changes per second without frame drops)
- **Rendering**: Canvas/WebGL for waveforms, automation lanes, and piano roll (DOM is too slow for these)
- **File I/O**: Native file system access for project files, audio files, presets
- **Plugin Hosting**: VST3/AU/CLAP SDK integration via native bridge

### 1.2 Audio Engine Requirements
- Sample rates: 44.1kHz, 48kHz, 88.2kHz, 96kHz, 176.4kHz, 192kHz
- Bit depths: 16-bit, 24-bit, 32-bit float
- Buffer sizes: 32 to 2048 samples (user configurable)
- Target latency: < 5ms round-trip at 128 samples/44.1kHz
- Processing: 64-bit double-precision floating point internal summing
- Thread model: Multi-threaded audio graph with lock-free message passing
- Zero-crossing detection for click-free editing
- Automatic delay compensation (PDC) across the entire signal chain
- Sample-accurate automation
- Offline rendering (faster-than-realtime bounce)

### 1.3 File Format Support
- **Audio Import**: WAV, AIFF, FLAC, OGG Vorbis, MP3, AAC, WMA
- **Audio Export**: WAV (16/24/32-bit), AIFF, FLAC, OGG, MP3 (via LAME)
- **MIDI**: Standard MIDI File (.mid) import/export
- **Project**: Custom project file format (.daw) — JSON + binary audio pool
- **Video**: Basic video import for scoring (MP4, MOV) — audio extraction + video playback sync

---

## 2. DUAL-VIEW SYSTEM (Core Differentiator)

### 2.1 Session View (Clip Launcher)
This is the non-linear, performance-oriented view — Ableton's signature feature.

**Grid Layout:**
- Vertical columns = Tracks (Audio, MIDI, Return, Master)
- Horizontal rows = Scenes
- Each cell = a Clip Slot (can hold one audio or MIDI clip)
- Empty slots show a "stop" button per track
- Clip slots display: clip name, color, playing status, launch button

**Clip Launching:**
- Click a clip to launch it (starts playing in sync with global quantization)
- Launch quantization options: None, 1/32, 1/16, 1/8, 1/4, 1/2, 1 Bar, 2 Bars, 4 Bars, 8 Bars
- Visual countdown indicator showing when clip will actually start
- Clips in the same track are mutually exclusive (launching one stops the previous)
- Clips across different tracks play simultaneously (layering)

**Scene Launching:**
- Click a Scene number/name to launch ALL clips in that row simultaneously
- Scene launch respects individual clip quantization settings
- Scenes can be named and colored
- Scene follow actions: trigger next scene, previous, first, last, any, or specific scene after N bars/beats

**Clip Stop Behavior:**
- Per-track stop buttons in empty slots
- Master stop-all-clips button
- Stop buttons respect quantization

**Recording into Session View:**
- Arm a track → press record → new clip is created in the selected slot
- Fixed-length recording: set clip length beforehand (1, 2, 4, 8 bars, etc.)
- Open-ended recording: records until you press stop
- MIDI overdub: layers new notes onto existing MIDI clips while looping

### 2.2 Arrangement View (Linear Timeline)
This is the traditional, left-to-right timeline for arranging full songs.

**Timeline:**
- Horizontal timeline with bar/beat/sub-beat ruler
- Time display: Bars.Beats.Sixteenths or Minutes:Seconds:Milliseconds (toggle)
- Zoom: horizontal (time) and vertical (track height) independently
- Scroll: smooth horizontal/vertical scrolling with trackpad/scroll wheel
- Pinch-to-zoom support

**Arrangement Clips:**
- Clips placed on tracks along the timeline
- Drag to move, resize (trim start/end), duplicate, split, consolidate
- Clip fades: fade-in, fade-out, crossfade between adjacent clips
- Fade curves: linear, logarithmic, exponential, S-curve (all adjustable)
- Clip gain: per-clip volume adjustment
- Clip transpose: per-clip pitch shift (semitones + cents)

**Arrangement Recording:**
- Real-time recording from Session View into Arrangement (captures a live performance)
- Overdub recording: layers on top of existing arrangement content
- Punch-in/punch-out recording between locators
- Loop recording with take lanes (comping)

**Comping (Take Lanes):**
- Multiple takes recorded into expandable lanes within a single track
- Click on segments of different takes to assemble the best composite
- Audition individual takes in isolation
- Flatten comped result into a single clip

**Navigation:**
- Locators: numbered markers on the timeline for quick navigation
- Loop brace: set loop region on the timeline (draggable)
- Cue points: jump to specific positions
- Keyboard shortcuts: Home (go to start), End (go to end), arrow keys for nudging

### 2.3 View Switching
- Tab key toggles between Session and Arrangement views
- Session-to-Arrangement recording: performing in Session View records all clip launches into the Arrangement
- "Back to Arrangement" button: when Session View playback overrides the Arrangement, one click returns to Arrangement playback
- Both views share the same tracks, mixer, and device chain

---

## 3. TRACK SYSTEM

### 3.1 Track Types

**Audio Tracks:**
- Record and play back audio
- Input monitoring: Auto, On, Off
- Input source selection: external audio inputs, resampling from other tracks
- Warp engine for time-stretching (see Section 7)
- Reverse audio playback
- Track freeze: render effects in place to save CPU (reversible)
- Track flatten: commit frozen track to audio permanently

**MIDI Tracks:**
- Record and play back MIDI data
- Host virtual instruments (built-in or VST/AU plugins)
- MIDI routing: input from MIDI controllers, other tracks, or virtual keyboards
- Multi-output instrument support (e.g., drum machines with separate outs)
- MPE (MIDI Polyphonic Expression) support
- MIDI channel filtering

**Return Tracks (Aux/Bus):**
- Receive audio via sends from other tracks
- Process with shared effects (reverb, delay, etc.)
- Pre-fader or post-fader send options
- Unlimited return tracks

**Group Tracks:**
- Nest multiple tracks inside a collapsible group
- Group has its own mixer channel (volume, pan, effects)
- Solo/mute group affects all children
- Nested groups supported (groups within groups)

**Master Track:**
- Final summing bus for all audio output
- Master effects chain (limiting, EQ, metering)
- Master volume and pan
- Always visible at the far right of the mixer

### 3.2 Track Properties
- Name (editable, double-click)
- Color (customizable from palette or custom hex)
- Height (adjustable per-track in Arrangement View)
- Arm for recording (button)
- Solo / Mute / Solo-in-Place
- Input/Output routing
- Monitor mode (In/Auto/Off)
- Track delay (positive or negative ms/samples for manual PDC adjustment)

### 3.3 Track Operations
- Create, duplicate, delete tracks
- Drag to reorder
- Insert track between existing tracks
- Select multiple tracks for batch operations
- Copy/paste tracks between projects
- Import tracks from other project files
- Default track templates

---

## 4. CLIP SYSTEM

### 4.1 Audio Clips
- Waveform display (accurate, zoomable)
- Start/End markers (trim points)
- Loop start/Loop end (independent of clip boundaries)
- Loop on/off toggle
- Clip gain (-inf to +36 dB)
- Clip transpose (-48 to +48 semitones)
- Clip detune (-50 to +50 cents)
- Warp on/off (see Section 7)
- Warp markers (anchor points for time-stretching)
- Warp modes: Beats, Tones, Texture, Re-Pitch, Complex, Complex Pro
- Reverse toggle
- RAM mode (load entire clip into RAM vs. stream from disk)
- Fade in/out (within clip, independent of arrangement fades)
- Clip envelope: per-clip automation for volume, pan, transpose, etc.

### 4.2 MIDI Clips
- Piano roll display (see Section 8)
- Note data: pitch, velocity, duration, position
- Per-note probability (0-100% chance of playing)
- Per-note velocity range (random velocity within range)
- MPE data: per-note pitch bend, slide, pressure
- Loop settings (same as audio clips)
- Clip-level MIDI effects
- Groove assignment (see Section 9)
- Legato mode
- Clip launch modes: Trigger, Gate, Toggle, Repeat

### 4.3 Clip Launch Modes
- **Trigger**: Press to start, plays until end or until another clip is launched
- **Gate**: Plays only while button is held down
- **Toggle**: Press to start, press again to stop
- **Repeat**: Continuously re-triggers at the launch quantization rate

### 4.4 Follow Actions
- After a clip finishes (or after N bars/beats), automatically trigger:
  - Next clip, Previous clip, First clip, Last clip
  - Any clip (random), Other clip (random, but not self)
  - No action, Stop
- Two follow actions with probability weighting (e.g., 80% next, 20% random)
- Follow action time: configurable in bars/beats/sixteenths
- Linked/Unlinked follow actions (relative to clip length or absolute time)

---

## 5. MIXER

### 5.1 Channel Strip (per track)
- Volume fader: -inf to +6 dB, high-resolution (0.1 dB steps)
- Pan knob: -100L to 100R (or stereo pan with width control)
- Stereo pan mode vs. Split stereo pan mode (independent L/R)
- Peak level meter (stereo, with peak hold and clip indicator)
- RMS/LUFS metering option
- Sends: unlimited send knobs to Return tracks (pre/post fader toggle per send)
- Solo button (with solo-safe option for returns)
- Mute button (cuts audio, different from deactivating the track)
- Arm button (record enable)
- Track activator (power on/off — deactivated track uses zero CPU)
- Input/Output selector dropdowns
- Device chain (insert effects) — shown as colored rectangles in the channel strip

### 5.2 Crossfader
- Assignable A/B crossfader (like a DJ mixer)
- Each track can be assigned to A, B, or neither
- Crossfader curve: constant power, linear, or slow fade
- Crossfader position: continuous control from A to B
- MIDI-mappable

### 5.3 Mixer Views
- Expanded mixer at the bottom of the screen (always visible)
- Full-screen mixer view
- I/O section (show/hide)
- Sends section (show/hide)
- Returns section (show/hide)
- Crossfader section (show/hide)

### 5.4 Routing
- Flexible audio routing: any track output can be routed to any other track input
- Sidechain routing: route audio from one track to a compressor/gate on another
- External audio: route to/from hardware audio interface channels
- Resampling: record the output of any track or the master into a new track
- MIDI routing: route MIDI between tracks, from external controllers, or from plugins

---

## 6. BUILT-IN INSTRUMENTS

### 6.1 Synthesizers

**Wavetable Synthesizer:**
- 2 wavetable oscillators with morphable wavetable position
- Sub oscillator (sine, triangle, square, noise)
- Wavetable editor: import custom wavetables or draw your own
- Unison mode: up to 16 voices per oscillator with detune and spread
- 2 multimode filters: LP, HP, BP, Notch, Formant, Morph (12/24 dB per octave)
- Filter FM from oscillators
- 3 envelopes (ADSR): Amp, Filter, Mod (with slope/curve per segment)
- 3 LFOs: multiple shapes, sync to tempo, fade-in, phase offset
- Modulation matrix: any source to any destination with amount and curve
- Built-in effects: chorus, flanger, phaser, reverb, delay, compressor, EQ, distortion
- 256+ factory presets

**Analog-Modeled Synthesizer:**
- 2 analog-modeled oscillators: Saw, Pulse (PWM), Sine, Noise
- Hard sync, oscillator FM
- Sub oscillator
- Multimode filter: LP, HP, BP, Notch (Ladder and State Variable models)
- Filter drive/saturation
- 2 envelopes (AHDSR) with retrigger options
- 2 LFOs with tempo sync
- Vibrato section
- Global unison and glide/portamento
- Output section with volume envelope and pan

**FM Synthesizer (Operator):**
- 4 operators (oscillators) with selectable waveforms (sine, saw, square, triangle, noise, custom)
- 11 FM algorithms (routing configurations)
- Per-operator: frequency ratio, fine tune, level, envelope, LFO
- Per-operator envelope: AHDSR with rate and level per segment, loop modes
- Global filter: LP, HP, BP, Notch (12/24 dB)
- Global LFO with multiple shapes
- Pitch envelope
- Glide/portamento
- Spread (stereo widening)

**Drift (Vintage Analog):**
- Voice architecture inspired by vintage analog monosynths
- 2 oscillators with shape morphing
- Analog-style instability/drift (pitch, filter, amplitude)
- LP filter with resonance and drive
- Cycling envelope (can function as LFO)
- Modulation routing via mod matrix
- Voice modes: Mono, Poly, Unison

**Collision (Physical Modeling):**
- Mallet/Noise exciter hitting a resonator
- Resonator types: Beam, Marimba, String, Membrane, Plate, Pipe, Tube
- Mallet stiffness, noise color, volume
- Resonator: material, decay, tune, brightness, inharmonics
- Built-in LFO and MIDI velocity mapping

**Tension (Physical Modeling String):**
- String exciter: bow, hammer, pluck
- String properties: decay, damping, vibrato, inharmonics
- Body resonance: size, type, decay
- Termination (bridge/nut modeling)
- Filter and damper sections

**Electric (Electric Piano):**
- Models classic electric pianos (Rhodes, Wurlitzer, etc.)
- Mallet: stiffness, force, noise
- Fork/Tine: color, decay, volume, tone
- Damper modeling
- Pickup position and type
- Global effects: tremolo, chorus, EQ

### 6.2 Samplers

**Simpler:**
- One-shot sample playback with loop
- Classic mode: standard sample playback with filter and envelope
- 1-Shot mode: plays sample once, ignores note-off
- Slicing mode: chop sample by transient/beat/region/manual, assign slices to MIDI notes
- Warp integration (same as audio clip warping)
- Filter: LP, HP, BP, Notch (12/24 dB), filter envelope
- Amp envelope (AHDSR)
- LFO with multiple destinations
- Pitch envelope
- Glide, spread, pan random
- Reverse, crop

**Sampler (Advanced Multi-Sample):**
- Multi-sample zones mapped across keyboard and velocity ranges
- Zone editor: visual mapping of samples to key/velocity ranges
- Up to 128 samples per instrument
- Round-robin and random sample selection
- Per-zone: root key, tune, gain, pan, filter, sample start/end, loop, crossfade
- 3 oscillators: can mix sample playback with synthesis oscillators
- Filter section: 2 serial/parallel filters, morph between types
- 3 modulation envelopes (AHDSR with curves)
- 3 LFOs (tempo-syncable)
- Modulation matrix with extensive source/destination options
- MIDI control mapping

**Drum Rack:**
- 128 pad grid (visible 16 at a time in 4x4 grid)
- Each pad = an independent instrument chain (can hold any instrument + effects)
- Choke groups: pads can mute each other (e.g., open/closed hi-hat)
- Pad routing: each pad can have its own audio output for individual processing
- Pad properties: tune, volume, pan, send levels
- Drag-and-drop samples onto pads
- Copy/paste/swap pads
- Layer multiple samples per pad (using instrument racks within pads)
- Receive MIDI notes: each pad responds to a specific MIDI note
- Slice-to-drum-rack: automatically chop an audio sample and map slices to pads

### 6.3 Utility Instruments

**External Instrument:**
- Route MIDI out to external hardware synths
- Route audio back in from hardware
- Automatic latency compensation
- Hardware I/O channel selection

**Instrument Rack:**
- Container that holds multiple instruments in parallel (layer) or switched (velocity/key zones)
- Chain selector: crossfade between multiple instruments based on a single macro knob
- Key zone editor: split keyboard across different instruments
- Velocity zone editor: different instruments for different velocities
- Up to 16 macro knobs for controlling parameters across all chains
- Macro mapping: any parameter in any chain can be mapped to a macro
- Macro ranges and curves (min/max, linear/log)
- Randomize button: create random macro mappings for sound design exploration

---

## 7. WARPING & TIME-STRETCHING

### 7.1 Warp Engine
- Real-time time-stretching without pitch change (and vice versa)
- Warp markers: manually anchor specific points in the audio to specific beats
- Auto-warp: automatically detect tempo and place warp markers
- Tempo detection: analyze audio file BPM
- Warp modes:
  - **Beats**: Best for rhythmic material. Preserves transients. Granularity: 1/32, 1/16, 1/8, 1/4, 1/2, 1 Bar, Transients. Back/forth loop envelope.
  - **Tones**: Best for monophonic melodic material. Grain size control.
  - **Texture**: Best for textures, pads, ambient. Grain size and flux controls.
  - **Re-Pitch**: Classic turntable-style speed change (pitch changes with tempo). No artifacts.
  - **Complex**: High-quality algorithm for full mixes and complex material. CPU intensive.
  - **Complex Pro**: Highest quality. Formant and envelope controls for preserving vocal character.

### 7.2 Beat Matching
- All audio clips warp to the global project tempo automatically
- Tempo changes in the arrangement: tempo automation
- Gradual tempo changes (ritardando, accelerando) with automation curves
- Half-time / double-time buttons for quick tempo manipulation of clips

---

## 8. MIDI EDITING (Piano Roll)

### 8.1 Note Editor
- Piano keyboard on the left (clickable for auditioning)
- Time grid on top (bars/beats)
- Grid snap: 1/32, 1/16T, 1/16, 1/8T, 1/8, 1/4T, 1/4, 1/2, 1 Bar, Off (adaptive based on zoom)
- Triplet grid option
- Dotted grid option
- Draw mode: click to create notes, drag to set length
- Select mode: click/drag to select notes, lasso selection
- Multi-select: Shift+click or Ctrl/Cmd+click
- Move notes: drag selected notes (constrained to grid or free)
- Resize notes: drag left/right edges
- Duplicate notes: Ctrl/Cmd+D or Alt+drag
- Delete notes: Select + Delete or double-click
- Velocity: shown as color intensity on notes + velocity lane at bottom
- Velocity editor: bar graph below piano roll, drag to adjust per-note velocity
- Velocity draw tool: draw velocity curves across multiple notes

### 8.2 MIDI Editing Tools
- Pencil tool: draw notes
- Selection tool: select and move notes
- Split tool: divide notes at click point
- Stretch tool: time-stretch a selection of notes proportionally
- Quantize: snap notes to grid (with amount: 0-100%)
- Quantize strength: partial quantization (move partway to grid)
- Iterative quantize: repeatedly approach the grid
- Humanize: add random timing/velocity variations
- Legato: extend each note to reach the next note
- Duplicate loop: duplicate selected notes at the end of the selection
- Transpose: up/down by semitone or octave
- Scale awareness: highlight notes in a selected scale, fold piano roll to show only in-scale notes
- Chord stamp: insert chord shapes with one click
- Note probability: set per-note probability (0-100%) for generative variation
- MIDI stretch markers: stretch/compress a region of MIDI notes proportionally

### 8.3 Automation Lanes (within MIDI clips)
- Velocity
- Pitch bend
- Mod wheel (CC1)
- Aftertouch (channel and polyphonic)
- Any MIDI CC (0-127)
- Per-note expression (MPE): pitch bend, slide, pressure per note
- Breakpoint editing: click to create points, drag to adjust, curves between points

---

## 9. GROOVE ENGINE

### 9.1 Groove Pool
- Drag groove templates to the Groove Pool
- Apply grooves to any clip (audio or MIDI)
- Extract groove from any audio/MIDI clip
- Factory groove library: MPC swing patterns, various feel templates

### 9.2 Groove Parameters
- Base: 1/4, 1/8, 1/16, 1/32 (the grid resolution the groove affects)
- Quantize amount: 0-100% (how much to snap to the groove grid)
- Timing amount: 0-100% (how much timing variation to apply)
- Random amount: 0-100% (additional random timing)
- Velocity amount: 0-100% (how much velocity variation to apply)
- Global groove amount: master control affecting all grooves in the project
- Commit groove: permanently apply the groove to the clip's notes

---

## 10. AUDIO EFFECTS (Built-In)

### 10.1 Dynamics

**Compressor:**
- Threshold, Ratio, Attack, Release, Knee
- Makeup gain (auto or manual)
- Sidechain input with EQ (HP/LP/BP filter on sidechain signal)
- Operating modes: Peak, RMS, Expand
- Compression models: Clean (FF1), Classic (FF2), Vintage (FB)
- Visual gain reduction meter and transfer curve display
- Dry/Wet mix (parallel compression)
- Lookahead

**Glue Compressor (Bus Compressor):**
- Modeled after SSL G-Bus compressor
- Threshold, Ratio (2:1, 4:1, 10:1), Attack (0.01 to 30ms), Release (0.1 to 1.2s, Auto)
- Makeup gain
- Range (compression ceiling)
- Dry/Wet mix
- Sidechain with HP filter
- Peak clip soft-clipper

**Multiband Dynamics:**
- 3-band dynamics processor (crossover frequencies adjustable)
- Per-band: compressor (above threshold) and expander (below threshold)
- Per-band: threshold, ratio, attack, release, gain
- Per-band: solo and mute
- Global output gain
- Sidechain input
- Visualization: input/output levels per band

**Gate:**
- Threshold, Attack, Hold, Release, Floor
- Sidechain input with EQ filter
- Flip mode (duck instead of gate)
- Return threshold (hysteresis)
- Lookahead

**Limiter:**
- Ceiling, Gain
- Lookahead
- Release (auto or manual)
- Stereo link
- Metering: input, output, gain reduction, true peak

### 10.2 EQ & Filters

**EQ Eight (Parametric EQ):**
- 8 bands, each selectable type: LP, HP, Low Shelf, High Shelf, Bell, Notch
- Per-band: frequency, gain, Q/bandwidth
- Filter slopes: 12, 24, 48 dB/octave
- Spectrum analyzer overlay (pre, post, or pre+post)
- Audition mode: solo a band to hear what it's affecting
- Oversampling: 1x, 2x, 4x (for high-frequency accuracy)
- Adaptive Q option
- Mid/Side processing mode
- Left/Right processing mode

**EQ Three (DJ-style EQ):**
- 3 fixed bands: Low, Mid, High
- Gain per band (-inf to +6 dB)
- Kill switches per band (instant mute)
- Crossover frequencies: adjustable Low/Mid and Mid/High
- 24 or 48 dB/octave slope

**Auto Filter:**
- Filter types: LP, HP, BP, Notch, Morph
- Filter models: Clean, OSR, MS2, SMP, PRD
- Frequency, Resonance
- LFO modulation: rate, amount, shape (sine, triangle, square, saw, random, S&H)
- Envelope follower: amount, attack, release
- Sidechain input for envelope follower
- Drive
- Tempo sync for LFO

**Channel EQ:**
- Simple 3-band EQ (Lo, Mid, Hi) + HP filter
- Quick and CPU-efficient
- Spectrum display
- Gain and frequency per band

### 10.3 Delay & Reverb

**Delay:**
- Sync mode (tempo-synced) or Free mode (milliseconds)
- Left and right delay times (independent or linked)
- Feedback amount
- Filter: HP and LP on feedback path
- Ping-pong mode
- Dry/Wet mix
- Modulation: rate and amount (chorus-like delay)
- Freeze button (infinite feedback)
- Repitch mode: changing delay time changes pitch (tape-style)

**Echo (Advanced Delay):**
- Dual delay lines with independent times
- Channel modes: Stereo, Ping Pong, Mid/Side
- Input HP/LP filters
- Feedback with HP/LP filter in feedback path
- Modulation: rate, amount, phase, filter mod
- Character: ducking, noise, wobble, gate
- Reverb on the delay tail
- Dry/Wet mix
- Envelope and ducking controls

**Reverb:**
- Decay time, Pre-delay, Room size
- High and low frequency damping (shelving EQ on reverb tail)
- Early reflections: shape, spin amount and rate
- Diffusion: amount and scale
- Chorus amount in the reverb tail
- Input HP/LP filter
- Stereo width
- Freeze mode
- Dry/Wet mix
- Quality modes: Eco, Mid, High

**Convolution Reverb (Hybrid Reverb):**
- Convolution engine: load impulse responses (IR files)
- Algorithmic reverb blended with convolution
- IR editor: start, end, fade, stretch
- EQ on reverb signal
- Modulation
- Routing: series or parallel between convolution and algorithmic
- IR library: rooms, halls, plates, springs, creative spaces

### 10.4 Modulation Effects

**Chorus-Ensemble:**
- Classic chorus with 2-3 modulated delay lines
- Rate, Amount (depth), Feedback
- HP/LP filter on wet signal
- Modes: Classic Chorus, Ensemble, Vibrato
- Dry/Wet mix
- Stereo spread

**Phaser:**
- All-pass filter sweep
- Poles: 2, 4, 6, 8 (number of filter stages)
- Rate (sync or free), Amount, Center Frequency, Feedback, Spread
- LFO shape: sine, triangle
- Env follower modulation
- Spin (stereo LFO offset)
- Dry/Wet

**Flanger:**
- Modulated short delay
- Rate, Amount, Feedback, Delay time
- HP/LP filter
- LFO shape
- Env follower modulation
- Dry/Wet

**Frequency Shifter:**
- Shift audio frequencies by a fixed Hz amount (not pitch shift)
- Coarse and Fine frequency controls
- Ring modulation mode
- Drive
- LFO with multiple shapes
- Dry/Wet

**Ring Modulator:**
- Multiply input signal with oscillator
- Oscillator: sine, square, saw, triangle, noise
- Frequency and fine tune
- LFO modulation of frequency
- Dry/Wet

### 10.5 Distortion & Saturation

**Saturator:**
- Multiple saturation curves: Analog Clip, Soft Sine, Medium Curve, Hard Curve, Sinoid Fold, Digital Clip
- Drive amount
- Base (DC offset)
- Color: tone controls (high/low EQ post-saturation)
- Output level
- Dry/Wet
- Waveshaper display

**Overdrive:**
- Modeled analog overdrive
- Drive, Tone, Dynamics (how much input dynamics are preserved)
- Bandwidth control
- Pre-drive filter

**Erosion:**
- Digital degradation: noise, sine, wide noise
- Frequency and amount
- Mode: wide noise, noise, sine
- Quality degradation

**Redux (Bitcrusher):**
- Bit depth reduction: 1-16 bits
- Sample rate reduction
- Quantization: hard, soft, random
- Filter: pre and post

**Pedal (Guitar Amp/Pedal Sim):**
- Overdrive, Distortion, Fuzz models
- Gain, Tone, Output
- Bass, Mid, Treble EQ
- Sub (low end boost)
- Dry/Wet

**Amp (Guitar Amplifier):**
- 7 amp models: Clean, Boost, Blues, Rock, Lead, Heavy, Bass
- Gain, Bass, Middle, Treble, Presence, Volume
- Cabinet simulation (integrated)

**Cabinet:**
- Speaker cabinet simulation
- Multiple cabinet types: 1x12, 2x12, 4x12, etc.
- Microphone position: near/far, on-axis/off-axis
- Dual mic blending

### 10.6 Utility & Routing

**Utility:**
- Gain (-inf to +36 dB)
- Stereo width (0% mono to 200% widened)
- Mono switch
- Left/Right balance
- Phase invert (L, R, or both)
- Bass mono: route low frequencies to mono below a threshold frequency
- DC offset filter
- Channel mode: Stereo, Left, Right, Swap, Mid, Side

**Tuner:**
- Chromatic tuner display
- Detects pitch of input audio
- Visual meter showing sharp/flat/in-tune
- Note name and octave display
- Reference frequency (A=440Hz adjustable)

**Spectrum:**
- Real-time FFT spectrum analyzer
- Linear or logarithmic frequency scale
- Block size options
- Channel: L, R, L+R, Mid, Side
- Min/Max range settings
- Display: line, bars, or filled

**Audio Effect Rack:**
- Container for chaining/parallel processing
- Parallel chains with individual volumes and chain selector
- Macro knobs (up to 16) mapped to any parameters
- Key zone and velocity zone routing
- Dry/Wet per chain

**External Audio Effect:**
- Send audio to an external hardware effect unit
- Route audio back in
- Gain compensation
- Latency compensation (auto-detected or manual)
- Hardware I/O selection

### 10.7 Spatial

**Stereo Panner:**
- Standard pan control
- Stereo balance mode
- Split stereo mode (independent L/R pan)
- Binaural panning option

**Spatial Audio:**
- Surround panning (up to 7.1.4)
- Dolby Atmos support (object-based panning)
- Binaural monitoring for headphones
- Height panning
- LFE send

---

## 11. MIDI EFFECTS

**Arpeggiator:**
- Styles: Up, Down, Up-Down, Down-Up, Converge, Diverge, Thumb, Pinky, Random, Played Order
- Rate: 1/1 to 1/32 (including triplets and dotted)
- Gate: note length as percentage of step length (1-400%)
- Steps: 1-8 octave range
- Transpose per step (up to 8 steps with individual semitone offsets)
- Velocity: from MIDI input, fixed, random, or pattern
- Retrigger: on new note, on new chord, or free-running
- Offset: shift the pattern start
- Repeats: number of cycles before transposing

**Chord:**
- Add up to 6 additional notes to each input note
- Per-added-note: semitone offset and velocity adjustment
- Shift: transpose the input note

**Note Length:**
- Override incoming note lengths
- Trigger mode: note on triggers a fixed-length note (gate time adjustable)
- Release mode: note off triggers a note
- Decay/fade envelope
- Gate: as time or synced to tempo

**Pitch:**
- Transpose incoming MIDI notes
- Range: -128 to +128 semitones
- Lowest/Highest note pass-through range
- Random pitch offset

**Random:**
- Add random variation to pitch, velocity, length
- Probability control
- Scale-aware randomization
- Chance per note (0-100%)

**Scale:**
- Force notes to a musical scale
- Base note selection
- Scale type: Major, Minor, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian, Whole Tone, Diminished, Pentatonic, Blues, Chromatic, etc.
- Fold mode: quantize out-of-scale notes to nearest in-scale note

**Velocity:**
- Remap incoming velocities
- Curve types: linear, logarithmic, exponential, S-curve, fixed
- Out Low / Out High range
- Random range
- Drive (amplify velocity differences)
- Compand (compress/expand velocity range)

**MIDI Effect Rack:**
- Chain multiple MIDI effects in series or parallel
- Macro knobs for controlling multiple parameters
- Key/velocity zone routing

---

## 12. AUTOMATION

### 12.1 Arrangement Automation
- Every parameter in the entire project is automatable
- Automation lanes below each track (expandable)
- Breakpoint editing: click to create points, drag to adjust
- Line segments between points with optional curves
- Curve types: linear, convex, concave, S-curve (drag the curve between two points)
- Draw mode: freehand draw automation
- Grid-snapped drawing
- Shapes: sine, triangle, square, saw (draw entire shapes into automation lanes)
- Copy/paste automation across tracks and parameters
- Thin automation: reduce the number of breakpoints while preserving shape
- Lock automation envelopes (so they don't move when clips are moved)
- Show/hide automation lanes

### 12.2 Clip Automation (Clip Envelopes)
- Per-clip automation independent of arrangement automation
- Available for: volume, pan, transpose, any device parameter, sends, MIDI CCs
- Linked or unlinked to clip position
- Same editing tools as arrangement automation
- Clip automation is relative to arrangement automation (adds on top)

### 12.3 Modulation
- Parameter modulation (LFO, envelope, random) without recording automation
- Modulation sources per device parameter:
  - LFO: rate, shape, amount, phase, offset
  - Envelope: attack, release, amount
  - Random: rate, amount
  - MIDI: map to any MIDI CC
- Modulation shown as a shaded range around the parameter knob

---

## 13. RECORDING

### 13.1 Audio Recording
- Arm tracks for recording (multiple tracks simultaneously)
- Input monitoring: hear input signal through effects chain
- Record into Session View (clip slots) or Arrangement View (timeline)
- Punch-in/punch-out between loop locators
- Count-in: 1, 2, or 4 bars before recording starts
- Pre-roll: play N bars before the punch-in point
- Record at tempo: metronome (see Section 14)
- Monitor level: separate from recording level
- Auto-name recorded clips with track name and timestamp

### 13.2 MIDI Recording
- Same arming/monitoring as audio
- Overdub mode: layer new notes on top of existing MIDI clips
- Capture MIDI: retroactively capture MIDI input you just played (even without pressing record!)
- Step recording: input notes one at a time at the current grid resolution
- MIDI note preview: hear notes as you draw them in the piano roll
- Quantize-while-recording option

### 13.3 Looper
- Built-in Looper device (audio effect)
- Record, Overdub, Play, Stop, Undo, Clear
- Tempo follows first loop or locks to project tempo
- Multiply: double the loop length
- Half speed, reverse
- Feedback control for overdub decay
- Sync to project transport

---

## 14. TRANSPORT & TEMPO

### 14.1 Transport Controls
- Play / Stop (spacebar)
- Record button (starts/stops recording)
- Loop toggle (on/off)
- Punch-in / Punch-out toggles
- Back-to-arrangement button
- Follow playhead toggle (auto-scroll)
- Metronome toggle

### 14.2 Tempo
- BPM: 20-999 (fine control to 0.01 BPM)
- Tap tempo button (tap repeatedly to set tempo)
- Tempo nudge: +/- buttons for fine real-time adjustment
- Tempo automation: tempo changes over time in the arrangement
- Time signature: any numerator over 1, 2, 4, 8, 16 (e.g., 4/4, 3/4, 7/8, 5/4, etc.)
- Time signature changes at any point in the arrangement

### 14.3 Metronome
- Click sound: selectable from multiple click sounds
- Count-in options: None, 1 bar, 2 bars, 4 bars
- Click during playback: on/off
- Click during recording only: option
- Rhythm: quarter notes, eighth notes, sixteenth notes
- Custom rhythm pattern
- Volume: independent metronome level

### 14.4 Global Quantization
- Controls when clips start/stop in Session View
- Options: None, 1/32 to 8 bars
- Applies to clip launching, scene launching, and clip stopping

---

## 15. BROWSER & FILE MANAGEMENT

### 15.1 Browser Panel (Left Side)
- **Categories/Collections:**
  - Sounds (all instruments and presets)
  - Drums (all drum kits and one-shots)
  - Instruments (built-in instruments)
  - Audio Effects
  - MIDI Effects
  - Max for Live (if applicable)
  - Plug-ins (VST/AU/CLAP)
  - Clips (saved clips with devices)
  - Samples (audio files)
  - Packs (installed content packs)

- **Places:**
  - Current Project folder
  - User Library (user presets and custom content)
  - Custom folders (user-added file system locations)

### 15.2 Search
- Full-text search across all content
- Search by name, tag, creator, pack
- Filter by type (instrument, effect, sample, preset)
- Filter by characteristics (e.g., "warm", "bright", "bass", "pad", "lead")
- Recent searches history
- Preview: click a sound in the browser to audition it instantly

### 15.3 Preview
- Audio file preview: plays the sample with automatic tempo matching
- Instrument preset preview: plays a default note/chord to audition
- Preview volume: independent level control
- Preview output: can route preview to a different output (cue)

### 15.4 Content Management
- Install/manage content packs
- Download additional packs from a store
- Create and save custom presets
- Tag and organize presets
- Export/import user library

---

## 16. PLUGIN HOSTING

### 16.1 Supported Formats
- VST3
- Audio Units (AU) — macOS
- CLAP (new open standard)
- VST2 (legacy support)

### 16.2 Plugin Management
- Auto-scan for installed plugins
- Manual rescan option
- Plugin blocklist (for crashing/incompatible plugins)
- Plugin folder management
- Plugin delay compensation (automatic)
- Plugin window: open plugin GUI in floating window
- Multiple plugin windows open simultaneously
- Plugin parameter automation (all parameters accessible)
- Plugin preset management (save/load from browser)
- Sandboxed plugin hosting (crash protection — if a plugin crashes, the DAW stays running)

### 16.3 Plugin Performance
- Individual plugin CPU metering
- Freeze track: render plugin output to audio to save CPU
- Plugin latency reporting
- Multi-threaded plugin processing

---

## 17. MAPPING & CONTROLLERS

### 17.1 MIDI Mapping Mode
- Enter MIDI Map mode (Ctrl/Cmd+M)
- Click any parameter in the GUI
- Move a knob/fader/button on your MIDI controller
- Mapping is created instantly
- Per-mapping: min/max range, takeover mode (pickup, jump, scale)
- Save/load MIDI mapping presets
- Multiple parameters mapped to one control (and vice versa)

### 17.2 Key Mapping Mode
- Enter Key Map mode (Ctrl/Cmd+K)
- Click any parameter → press a key on the computer keyboard
- Toggle buttons, trigger clips, control transport
- Key range assignments for instruments

### 17.3 Control Surface Support
- Auto-detection of popular MIDI controllers
- Custom control surface scripts (Python/Lua)
- Dedicated support for:
  - Ableton Push (or custom grid controller equivalent)
  - Novation Launchpad
  - Akai APC40/APC Mini
  - Native Instruments Maschine
  - Generic MIDI controllers
- Control surface locking: lock a controller to a specific device

### 17.4 OSC Support
- Open Sound Control protocol for network-based control
- TouchOSC and similar tablet controllers
- Custom OSC mapping

---

## 18. EXPORT & RENDERING

### 18.1 Audio Export
- Export selection or entire arrangement
- Rendered track: Master, Individual tracks, All tracks (stems), Selected tracks only
- File format: WAV, AIFF, FLAC, OGG, MP3
- Bit depth: 16, 24, 32-bit float
- Sample rate: match project or convert (44.1k through 192k)
- Dither: Off, Triangular, POW-r 1/2/3 (for bit depth reduction)
- Normalize: on/off
- Render as loop: seamless loop export
- Create analysis file
- Convert to mono
- Offline rendering (faster than real-time)
- Include return/master effects: toggle

### 18.2 MIDI Export
- Export MIDI clips as Standard MIDI Files (.mid)
- Export individual track or all tracks

### 18.3 Stem Export
- Export each track as a separate audio file simultaneously
- Naming: track name + project name
- Include/exclude return and master processing per stem

### 18.4 Video Export
- Export audio synced with imported video
- Video codec options
- Resolution matching

---

## 19. PROJECT MANAGEMENT

### 19.1 Project Structure
- Project file (.daw) contains all settings, MIDI data, automation, device settings
- Audio pool: collected audio files stored alongside the project
- "Collect All and Save": gather all referenced audio into the project folder
- Backup versions: auto-save with version history
- Project templates: save and load project templates with tracks, routing, devices pre-configured
- Default project template (loads on startup)

### 19.2 Undo History
- Unlimited undo/redo
- Undo history panel: see all actions, click to jump to any point
- Undo for EVERY action: parameter changes, clip edits, device changes, mixer changes
- Branching undo (preserving alternate histories)

### 19.3 CPU & Performance
- CPU load meter (real-time)
- Disk activity meter
- Per-track CPU usage
- Audio dropout indicator
- Performance preferences: multi-core processing, thread count, buffer size

---

## 20. USER INTERFACE DESIGN

### 20.1 Overall Layout

```
+-------+---------------------------------------------+
|       |          ARRANGEMENT / SESSION VIEW          |
|       |  +---------------------------------------+   |
|       |  |                                       |   |
| B  R  |  |     Main Canvas                       |   |
| R  O  |  |     (Timeline / Clip Grid)            |   |
| O  W  |  |                                       |   |
| W  S  |  +---------------------------------------+   |
| S  E  |                                              |
| E  R  |  +---------------------------------------+   |
| R     |  |     Detail View                       |   |
|       |  |     (Clip Editor / Device Chain /      |   |
|       |  |      Piano Roll / Mixer)               |   |
|       |  +---------------------------------------+   |
+-------+---------------------------------------------+
|              TRANSPORT BAR & INFO BAR                |
+-----------------------------------------------------+
```

### 20.2 Color Scheme
- Dark theme (default): deep grey/charcoal background (#1E1E1E), slightly lighter panels (#2D2D2D), bright accent colors for clips and UI elements
- High contrast text: white (#FFFFFF) on dark, with subtle grey (#999999) for secondary text
- Clip colors: vibrant, user-assignable palette (16+ colors)
- Metering: green → yellow → orange → red gradient
- Selection highlight: blue outline with subtle glow
- Waveform colors: match clip color with lighter/darker shading
- Light theme option

### 20.3 UI Components
- **Knobs**: Rotary with value display, mouse drag (vertical/circular), double-click to reset
- **Faders**: Vertical sliders with precise control, fine adjustment with Shift+drag
- **Buttons**: Toggle (lit when active) and momentary (lit while held)
- **Menus**: Dark context menus with keyboard accelerators
- **Tooltips**: Parameter name, current value, and range on hover
- **Text fields**: Click to edit numerical values directly
- **Scrollbars**: Thin, auto-hiding, draggable
- **Resize handles**: Between all major panels
- **Drag and drop**: Files, clips, devices, samples throughout the entire UI
- **Zoom**: Ctrl/Cmd+scroll for horizontal zoom, Ctrl/Cmd+Alt+scroll for vertical zoom

### 20.4 Responsive Layout
- All panels resizable by dragging borders
- Browser panel: show/hide (Ctrl/Cmd+Alt+B)
- Detail view: show/hide (Ctrl/Cmd+Alt+L)
- Mixer: show/hide (Ctrl/Cmd+Alt+M)
- Full-screen mode
- Second window support (e.g., plugins or mixer on second monitor)

---

## 21. KEYBOARD SHORTCUTS (Essential)

| Action | Shortcut |
|--------|----------|
| Play/Stop | Space |
| Record | F9 |
| Loop toggle | Ctrl/Cmd+L |
| Toggle Session/Arrangement | Tab |
| Show/Hide Browser | Ctrl/Cmd+Alt+B |
| Show/Hide Detail | Ctrl/Cmd+Alt+L |
| Show/Hide Mixer | Ctrl/Cmd+Alt+M |
| Draw mode | B |
| Duplicate | Ctrl/Cmd+D |
| Split clip | Ctrl/Cmd+E |
| Consolidate | Ctrl/Cmd+J |
| Quantize | Ctrl/Cmd+U |
| Undo | Ctrl/Cmd+Z |
| Redo | Ctrl/Cmd+Shift+Z |
| MIDI Map mode | Ctrl/Cmd+M |
| Key Map mode | Ctrl/Cmd+K |
| Add audio track | Ctrl/Cmd+T |
| Add MIDI track | Ctrl/Cmd+Shift+T |
| Add return track | Ctrl/Cmd+Alt+T |
| Group tracks | Ctrl/Cmd+G |
| Freeze track | Right-click → Freeze |
| Rename | Ctrl/Cmd+R |
| Delete | Delete/Backspace |
| Select all | Ctrl/Cmd+A |
| Zoom in | Ctrl/Cmd+= |
| Zoom out | Ctrl/Cmd+- |
| Zoom to fit | Ctrl/Cmd+Shift+F |
| Zoom to selection | Z |
| Save | Ctrl/Cmd+S |
| Save As | Ctrl/Cmd+Shift+S |
| Export Audio | Ctrl/Cmd+Shift+R |
| Preferences | Ctrl/Cmd+, |
| Tap tempo | T (in transport) |
| Follow playhead | Ctrl/Cmd+Shift+F |
| Toggle clip/device view | Shift+Tab |
| Arm all tracks | (custom) |
| Navigate clips | Arrow keys |
| Launch clip | Enter |
| Stop clip | . |

---

## 22. PREFERENCES

### 22.1 Audio
- Audio device selection (interface)
- Input/output configuration
- Sample rate
- Buffer size
- Driver type (CoreAudio, ASIO, WASAPI, ALSA, JACK)
- Test tone
- Input/output channel mapping

### 22.2 MIDI
- MIDI input/output devices
- Control surface configuration
- Takeover mode: Pickup, Jump, Relative
- MIDI clock sync: send/receive
- MIDI timecode (MTC): send/receive

### 22.3 File/Folder
- User Library location
- Default save location
- Temporary folder location
- Content pack installation folder
- Sample rate conversion quality
- Auto-save interval

### 22.4 Look/Feel
- Theme: Dark, Light, Custom
- Color scheme customization
- Zoom display: follow song position, auto-zoom
- Language
- HiDPI/Retina scaling
- Animation quality

### 22.5 CPU
- Multi-core processing toggle
- Number of processing threads
- Audio-to-CPU balance
- Process in 32-bit or 64-bit

---

## 23. ADVANCED FEATURES

### 23.1 Max for Live Equivalent (Extensibility)
- Visual programming environment for custom devices
- Node-based patching (audio, MIDI, control signals)
- Custom UI creation
- Access to all DAW parameters via API
- Community sharing of custom devices
- JavaScript/TypeScript scripting engine for device logic

### 23.2 Link (Network Sync)
- Sync tempo and phase with other music apps over local network
- Ableton Link protocol compatible
- Join/leave Link sessions
- Start/stop sync
- Works over Wi-Fi and wired networks

### 23.3 CV/Gate Output
- Control voltage output for modular synthesizers
- Gate signals for triggers
- Requires compatible audio interface (DC-coupled)
- Pitch CV, Gate, Velocity, Mod mappable to audio outputs

### 23.4 Collaboration
- Cloud project storage
- Real-time collaboration (multiple users editing the same project)
- Project sharing via link
- Version history with diff view
- Comments on timeline positions
- Chat within the application

### 23.5 AI-Powered Features (Beyond Ableton)
- AI stem separation (vocals, drums, bass, other) from mixed audio
- AI-assisted mixing (suggest EQ, compression, levels)
- AI chord detection from audio
- AI melody/harmony suggestion
- AI-powered mastering assistant
- Intelligent tempo/key detection
- Natural language search ("find me a dark pad sound")
- AI-generated drum patterns from text description
- Style transfer: apply the "feel" of one clip to another

---

## 24. PERFORMANCE TARGETS

| Metric | Target |
|--------|--------|
| Audio latency (round-trip) | < 5ms at 128 samples |
| UI frame rate | 60fps minimum |
| Track count | 200+ tracks without dropout |
| Plugin instances | 100+ simultaneous |
| Undo history | 10,000+ actions |
| Project load time | < 3 seconds (average project) |
| Audio file streaming | 100+ simultaneous streams from disk |
| CPU usage (idle) | < 2% |
| RAM usage (empty project) | < 200MB |
| Startup time | < 2 seconds |
| Waveform rendering | 60fps smooth scrolling at any zoom |
| MIDI jitter | < 1ms |

---

## 25. PLATFORM SUPPORT

- macOS (Apple Silicon native + Intel)
- Windows 10/11 (64-bit)
- Linux (Ubuntu, Fedora, Arch — JACK and PipeWire)

---

## 26. ACCESSIBILITY

- Screen reader support
- High-contrast mode
- Keyboard-only navigation (every action accessible without mouse)
- Resizable UI elements
- Color-blind friendly palette options
- Focus indicators on all interactive elements

---

## 27. IMPLEMENTATION PHASES

### Phase 1: Core Engine (Months 1-3)
- Audio engine: playback, recording, routing, mixing
- MIDI engine: input, output, recording, playback
- Transport: play, stop, record, loop, tempo, metronome
- Basic track system: audio tracks, MIDI tracks, master track
- Basic mixer: volume, pan, mute, solo
- File I/O: WAV/AIFF import/export, project save/load
- Basic arrangement view with timeline

### Phase 2: Editing & Clips (Months 4-6)
- Clip system: audio clips with waveform display, MIDI clips with piano roll
- Session View with clip grid and launching
- Clip editing: trim, move, duplicate, split, consolidate
- MIDI editing: full piano roll with all tools
- Audio warping engine (all warp modes)
- Basic automation (arrangement + clip)
- Undo/redo system

### Phase 3: Instruments & Effects (Months 7-10)
- Wavetable synth
- Analog synth
- FM synth
- Simpler + Sampler
- Drum Rack
- All dynamics processors (compressor, gate, limiter, multiband)
- All EQs and filters
- All delay and reverb effects
- All modulation effects
- All distortion effects
- All MIDI effects
- Utility devices

### Phase 4: Advanced Features (Months 11-14)
- Plugin hosting (VST3/AU/CLAP)
- MIDI mapping and controller support
- Groove engine
- Comping / take lanes
- Follow actions
- Group tracks and return tracks
- Audio effect racks and instrument racks
- Full export options (stems, video)
- Browser with search and preview

### Phase 5: Polish & Extended Features (Months 15-18)
- Visual programming / extensibility system
- Network sync (Link protocol)
- AI features (stem separation, mixing assistant, etc.)
- Collaboration features
- Content library (factory presets, samples, loops)
- Themes and customization
- Performance optimization
- Accessibility audit and improvements
- Cross-platform testing and packaging

---

## 28. PROJECT FILE FORMAT

### .daw Project Structure
```
MyProject/
├── MyProject.daw              # Main project file (JSON)
│   ├── project_metadata       # BPM, time sig, sample rate, version
│   ├── tracks[]               # Track definitions and ordering
│   ├── clips[]                # All clip data (MIDI notes, warp markers, etc.)
│   ├── devices[]              # All device chains and parameters
│   ├── automation[]           # All automation data
│   ├── mixer_state            # Volumes, pans, sends, routing
│   ├── transport_state        # Loop points, locators, tempo automation
│   └── mapping_state          # MIDI/key mappings
├── Audio/                     # Audio pool
│   ├── Samples/               # Imported audio files
│   └── Recordings/            # Recorded audio files
├── Presets/                   # Saved device presets
├── Backups/                   # Auto-save versions
└── Analysis/                  # Waveform cache, transient data
```

---

This specification covers every major feature needed to build a professional DAW matching Ableton Live's capabilities. Each section can be expanded into detailed engineering documents with API specifications, data models, and UI wireframes during implementation.
