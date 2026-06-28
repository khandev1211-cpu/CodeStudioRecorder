# 04 — Audio Engine

## CodeStudio Recorder — Audio Engine

---

## Table of Contents

1. [Overview](#overview)
2. [Audio Sources](#audio-sources)
3. [WASAPI Integration](#wasapi-integration)
4. [Microphone Capture](#microphone-capture)
5. [System Audio Capture (Loopback)](#system-audio-capture-loopback)
6. [Audio Mixing](#audio-mixing)
7. [Synchronization](#synchronization)
8. [Audio Effects](#audio-effects)
9. [Normalization](#normalization)
10. [Recording Pipeline](#recording-pipeline)
11. [Configuration Reference](#configuration-reference)

---

## Overview

The Audio Engine is responsible for capturing, processing, mixing, and delivering synchronized audio to the Encoding Engine. It handles two independent capture paths — **microphone input** and **system audio loopback** — and merges them into a single mixed PCM stream.

**Key goals:**
- Minimal latency from audio event to encoded packet
- Sample-accurate A/V synchronization
- Noise-robust recording (noise gate, normalization)
- Zero audio dropout under normal load

---

## Audio Sources

| Source | API | Mode | Notes |
|---|---|---|---|
| Microphone | WASAPI | Exclusive or Shared | User voice input |
| System Audio | WASAPI Loopback | Shared loopback | What you hear — game, music, notification |
| Virtual Mic (future) | VB-Cable / plugin | Shared | Third-party virtual audio routing |
| Webcam audio (future) | DirectShow | Shared | Combined webcam mic |

Users can independently enable/disable/volume-control each source.

---

## WASAPI Integration

All audio capture uses **Windows Audio Session API (WASAPI)** — the lowest-latency, highest-quality Windows audio API available to user-mode code.

### WASAPI Modes

**Shared Mode** — Used for loopback capture:
- Audio goes through Windows audio engine (sample rate conversion, mixing)
- Latency: ~10–20ms
- No exclusive device access

**Exclusive Mode** — Available for microphone:
- Direct device access, bypasses Windows audio engine
- Latency: ~1–3ms
- Requires negotiated buffer size with device

```cpp
// Initialize WASAPI client
ComPtr<IAudioClient> audioClient;
device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, 
                 (void**)&audioClient);

audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,      // or EXCLUSIVE
    AUDCLNT_STREAMFLAGS_LOOPBACK,  // loopback flag for system audio
    hnsBufferDuration,
    0, pwfx, nullptr
);
```

---

## Microphone Capture

### MicCapture Class

```cpp
class MicCapture {
public:
    AudioResult initialize(const MicConfig& config);
    void        start();
    void        stop();
    AudioStats  getStats() const;
    
    // Callback invoked per buffer (on capture thread)
    using FrameCallback = std::function<void(AudioBuffer)>;
    void setCallback(FrameCallback cb);

private:
    ComPtr<IAudioClient>        client_;
    ComPtr<IAudioCaptureClient> captureClient_;
    WAVEFORMATEXTENSIBLE        format_;
    std::thread                 captureThread_;
};
```

### Capture Loop

```cpp
void MicCapture::captureLoop() {
    while (running_) {
        UINT32 packetSize = 0;
        captureClient_->GetNextPacketSize(&packetSize);
        
        while (packetSize > 0) {
            BYTE* data;
            UINT32 frames;
            DWORD flags;
            UINT64 devicePosition;
            UINT64 qpcPosition;
            
            captureClient_->GetBuffer(&data, &frames, &flags,
                                      &devicePosition, &qpcPosition);
            
            if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                AudioBuffer buf{data, frames, qpcPosition, format_};
                callback_(buf);
            }
            
            captureClient_->ReleaseBuffer(frames);
            captureClient_->GetNextPacketSize(&packetSize);
        }
        
        WaitForSingleObject(hEvent_, 10); // 10ms wake interval
    }
}
```

---

## System Audio Capture (Loopback)

System audio loopback captures everything playing through the default audio output (speakers/headphones) — game audio, music, notification sounds, etc.

```cpp
class LoopbackCapture {
public:
    // Same interface as MicCapture
    // Key difference: initialized with AUDCLNT_STREAMFLAGS_LOOPBACK
    
    AudioResult initialize(const LoopbackConfig& config);
    void        start();
    void        stop();
};
```

### Loopback Initialization

```cpp
audioClient->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
    REFTIMES_PER_SEC / 100, // 10ms buffer
    0,
    pwfx,
    nullptr
);
```

### Silent Periods

When no system audio is playing, WASAPI loopback will stall (no callbacks). The engine detects this and inserts **silence buffers** to maintain the audio timeline:

```cpp
if (timeSinceLastPacket > silenceThresholdMs_) {
    injectSilence(timeSinceLastPacket);
}
```

---

## Audio Mixing

The `AudioMixer` combines microphone and system audio into a single stereo PCM stream delivered to the encoder.

```cpp
class AudioMixer {
public:
    void setMicVolume(float gain);           // 0.0 – 2.0 (1.0 = unity)
    void setSystemVolume(float gain);
    void setMicEnabled(bool enabled);
    void setSystemEnabled(bool enabled);
    
    AudioBuffer mix(const AudioBuffer& mic, const AudioBuffer& system);

private:
    float micGain_    = 1.0f;
    float systemGain_ = 1.0f;
    bool  micEnabled_    = true;
    bool  systemEnabled_ = true;
    
    void resampleIfNeeded(AudioBuffer& buf, uint32_t targetRate);
    void applyGain(AudioBuffer& buf, float gain);
    void mixFloat32(float* dst, const float* src, size_t frames);
};
```

### Sample Rate Harmonization

If microphone and system audio are at different sample rates (e.g. mic at 44100Hz, system at 48000Hz), the mixer resamples to the **target encoder rate** (default: 48000Hz) using linear interpolation or, for high quality mode, a Sinc resampler.

### Mixing Formula

```
output[i] = clamp(mic[i] * micGain + system[i] * systemGain, -1.0, 1.0)
```

Clipping is handled by a soft limiter applied after mixing.

---

## Synchronization

Audio samples must be aligned with video frames for correct A/V sync in the output file.

### Timestamp Strategy

- Every `AudioBuffer` carries a `qpcTimestamp` from WASAPI (`QPC100ns` device position)
- This timestamp is relative to the same QPC clock as video frame timestamps
- The `Muxer` uses these timestamps to interleave audio packets and video frames

### Buffer Size Tradeoffs

| Buffer Size | Latency | CPU Overhead | Risk |
|---|---|---|---|
| 5ms | Very low | High (frequent wakeups) | More context switching |
| 10ms | Low | Moderate | Balanced |
| 20ms | Moderate | Low | Slight sync imprecision |
| 50ms | High | Minimal | Noticeable desync risk |

Default: **10ms** buffer for recording (20ms for low-CPU mode).

---

## Audio Effects

### Noise Gate

Suppresses background noise below a configurable threshold:

```cpp
class NoiseGate {
public:
    void setThreshold(float dbFS);  // e.g. -40dBFS
    void setAttack(int ms);
    void setRelease(int ms);
    void process(AudioBuffer& buf);
};
```

When input level is below threshold → gain is attenuated to silence.

### High-Pass Filter

Removes low-frequency rumble (air conditioning, desk vibration):

```cpp
class HighPassFilter {
public:
    void setCutoff(float hz); // default: 80Hz
    void process(AudioBuffer& buf);
};
```

### Echo Cancellation (future, Phase 2)

Will use Windows AEC (Acoustic Echo Cancellation) via `IAudioProcessingObject` or WebRTC's AEC module to cancel system audio bleed into the microphone.

---

## Normalization

Audio normalization ensures consistent output levels across the recording.

### Peak Normalization (real-time)

Applied during mixing — prevents clipping:

```cpp
class PeakLimiter {
public:
    void setLookAhead(int ms);  // default: 5ms
    void setRelease(int ms);
    void process(AudioBuffer& buf);
};
```

### Loudness Normalization (post-processing, Phase 2)

After recording, the export pipeline optionally applies **EBU R128 loudness normalization** to target -14 LUFS (standard for YouTube):

```
ffmpeg -i input.mp4 -af loudnorm=I=-14:TP=-1.5:LRA=11 output.mp4
```

---

## Recording Pipeline

```
[Microphone Device]          [System Audio Device]
       │                              │
[MicCapture Thread]         [LoopbackCapture Thread]
       │                              │
       └────────────┬─────────────────┘
                    │
             [AudioMixer]
                    │
             [NoiseGate]  ← optional
                    │
             [HighPassFilter]  ← optional
                    │
             [PeakLimiter]
                    │
             [AudioQueue] ← lock-free SPSC
                    │
             [AudioEncoder]
                    │
             [Muxer] → Output File
```

---

## Configuration Reference

```cpp
struct AudioConfig {
    // Microphone
    bool     micEnabled        = true;
    float    micGain           = 1.0f;
    uint32_t micSampleRate     = 48000;
    uint16_t micChannels       = 1;       // mono mic
    bool     micNoiseGate      = true;
    float    micNoiseGateDb    = -40.0f;
    bool     micHighPass       = true;
    float    micHighPassHz     = 80.0f;

    // System Audio
    bool     systemEnabled     = true;
    float    systemGain        = 1.0f;

    // Output
    uint32_t outputSampleRate  = 48000;
    uint16_t outputChannels    = 2;       // stereo output
    
    // Buffer
    uint32_t bufferMs          = 10;
    
    // Encoder
    AudioCodec outputCodec     = AudioCodec::AAC;
    uint32_t   outputBitrate   = 192000; // 192 kbps
};
```

---

*Last updated: 2025 | Module 04 of 19*
