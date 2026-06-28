# 05 — Encoding Engine

## CodeStudio Recorder — Encoding Engine

---

## Table of Contents

1. [Overview](#overview)
2. [FFmpeg Integration](#ffmpeg-integration)
3. [Hardware Encoders](#hardware-encoders)
4. [Software Encoders](#software-encoders)
5. [Encoder Selection Logic](#encoder-selection-logic)
6. [Bitrate Control](#bitrate-control)
7. [Codec Reference](#codec-reference)
8. [Container Formats](#container-formats)
9. [Export Pipeline](#export-pipeline)
10. [Encoder Configuration Reference](#encoder-configuration-reference)

---

## Overview

The Encoding Engine converts raw video frames (GPU textures) and audio PCM samples into compressed, container-wrapped media files. It abstracts over multiple hardware and software encoders, presenting a unified interface to the Recording Engine.

**Key responsibilities:**
- Encoder capability probing and selection
- Real-time video and audio encoding
- Bitrate and quality management
- Container muxing
- Export preset management

---

## FFmpeg Integration

FFmpeg is used for:
- **Audio encoding** (AAC, Opus, MP3) via libavcodec
- **Container muxing** (MP4, MKV, WebM, etc.) via libavformat
- **Software video encoding** (libx264, libx265) via libavcodec
- **Post-processing exports** (GIF, trim, resize) via ffmpeg CLI

For **hardware video encoding**, native NVENC/QSV/AMF APIs are used directly (not through FFmpeg's hwaccel wrappers) for maximum performance and control.

### FFmpeg Build

The project ships a custom FFmpeg build:
- Stripped to required codecs only (no decoders needed for recording)
- LGPL compliance — dynamically linked
- Windows x64, targeting Windows 10 1903+

---

## Hardware Encoders

### NVENC (NVIDIA)

**API:** NVIDIA Video Codec SDK (nvEncodeAPI)

```cpp
class NVENCEncoder : public IVideoEncoder {
public:
    EncoderResult initialize(const VideoEncoderConfig& config);
    EncoderResult encodeFrame(const VideoFrame& frame);
    EncoderResult flush(std::vector<EncodedPacket>& out);
    void          destroy();

private:
    void*          encoder_;      // NV_ENC_OPEN_ENCODE_SESSION_EX
    NV_ENC_CONFIG  encodeConfig_;
    CUcontext      cudaContext_;
    
    // Input/output buffers
    std::vector<NV_ENC_INPUT_PTR>  inputBuffers_;
    std::vector<NV_ENC_OUTPUT_PTR> outputBuffers_;
};
```

**Supported profiles on NVENC:**
- H.264 (Baseline, Main, High)
- H.265 / HEVC (Main, Main10)
- AV1 (Ada Lovelace GPUs only — RTX 40xx+)

**Recommended settings for recording:**

| Quality | RC Mode | Bitrate | Notes |
|---|---|---|---|
| Speed | CBR | 8–15 Mbps | Lowest latency, slightly larger |
| Balanced | VBR | 8–20 Mbps | Default recommendation |
| Quality | CQ | QP 20–28 | Best quality, variable size |

---

### Intel Quick Sync (QSV)

**API:** Intel Media SDK / oneVPL

```cpp
class QuickSyncEncoder : public IVideoEncoder {
public:
    EncoderResult initialize(const VideoEncoderConfig& config);
    EncoderResult encodeFrame(const VideoFrame& frame);
    EncoderResult flush(std::vector<EncodedPacket>& out);

private:
    mfxSession   session_;
    mfxVideoParam videoParams_;
    
    MFXVideoENCODE encoder_;
    mfxFrameSurface1* surfaces_;
};
```

**Supported:** H.264, H.265, AV1 (12th Gen+ Intel CPUs)

**Quick Sync is preferred** when NVIDIA GPU is absent or when the system has a modern Intel iGPU alongside a discrete GPU (saves discrete GPU headroom for the running application).

---

### AMD AMF (Advanced Media Framework)

**API:** AMD Advanced Media Framework SDK

```cpp
class AMFEncoder : public IVideoEncoder {
    amf::AMFFactory*  factory_;
    amf::AMFContext*  context_;
    amf::AMFComponent* encoder_;
};
```

**Supported:** H.264, H.265, AV1 (RDNA 3+ GPUs)

---

## Software Encoders

Software encoding is used as a fallback when no hardware encoders are available, or for export presets requiring codecs not available in hardware.

### libx264 (H.264 Software)

```cpp
// FFmpeg + libx264 encoding
avcodec_find_encoder(AV_CODEC_ID_H264);

// Key options
av_opt_set(codecCtx->priv_data, "preset", "ultrafast", 0);
av_opt_set(codecCtx->priv_data, "tune",   "zerolatency", 0);
av_opt_set(codecCtx->priv_data, "crf",    "18", 0);
```

**Presets:**
| Preset | CPU Usage | File Size | Recommended For |
|---|---|---|---|
| ultrafast | Very low | Larger | Real-time, slow machines |
| fast | Low | Moderate | Default software fallback |
| medium | Moderate | Good | Post-process exports |
| slow | High | Small | Archive quality |

### libx265 (H.265 Software)

Used for high-quality exports. Not used in real-time recording on most machines due to CPU cost.

### libvpx-vp9 / libaom-av1

Used for WebM/web-optimized exports.

---

## Encoder Selection Logic

```cpp
class EncoderFactory {
public:
    std::unique_ptr<IVideoEncoder> createBestEncoder(
        const VideoEncoderConfig& config)
    {
        // 1. Check preference from config
        if (config.preferredEncoder != EncoderType::Auto) {
            return createEncoder(config.preferredEncoder, config);
        }
        
        // 2. Auto-select: probe hardware
        if (probeNVENC()) {
            return std::make_unique<NVENCEncoder>();
        }
        if (probeQuickSync()) {
            return std::make_unique<QuickSyncEncoder>();
        }
        if (probeAMF()) {
            return std::make_unique<AMFEncoder>();
        }
        
        // 3. Software fallback
        return std::make_unique<SoftwareEncoder>(SoftwareCodec::x264);
    }
    
private:
    bool probeNVENC();
    bool probeQuickSync();
    bool probeAMF();
};
```

### Probing Logic

Each probe:
1. Attempts to load the required DLL (`nvEncodeAPI64.dll`, `mfxhw64.dll`, `amfrt64.dll`)
2. Queries supported codecs and profiles
3. Attempts a minimal test encode (1 frame) to verify functional hardware
4. Returns `false` if any step fails

---

## Bitrate Control

### Rate Control Modes

| Mode | Use Case | Description |
|---|---|---|
| CBR | Streaming, upload-limited | Constant bitrate — predictable file size |
| VBR | General recording | Variable within min/max range |
| CQP | Quality-first | Constant QP — hardware-specific |
| CRF | Software encoding | Constant Rate Factor (libx264/x265) |

### Bitrate Recommendations

| Resolution | FPS | Recommended Bitrate (H.264) |
|---|---|---|
| 720p | 30 | 4–8 Mbps |
| 1080p | 30 | 6–12 Mbps |
| 1080p | 60 | 8–20 Mbps |
| 1440p | 60 | 16–30 Mbps |
| 4K | 60 | 35–60 Mbps |

H.265 achieves similar quality at ~50% lower bitrate.

---

## Codec Reference

| Codec | ID | Hardware | Software | Container | Notes |
|---|---|---|---|---|---|
| H.264 | AVC | NVENC, QSV, AMF | libx264 | MP4, MKV | Universal support |
| H.265 | HEVC | NVENC, QSV, AMF | libx265 | MP4, MKV | Better quality, less support |
| AV1 | AV1 | NVENC (Ada), QSV (12th+) | libaom | MP4, MKV, WebM | Future-proof, slow SW |
| VP9 | VP9 | — | libvpx-vp9 | WebM | Web optimized |
| GIF | — | — | FFmpeg | .gif | Short clips only |
| AAC | AAC | — | libfdk-aac | MP4, MKV | Default audio |
| Opus | Opus | — | libopus | MKV, WebM | High quality audio |

---

## Container Formats

| Container | Extension | Video | Audio | Notes |
|---|---|---|---|---|
| MPEG-4 | .mp4 | H.264, H.265, AV1 | AAC | Universal, recommended default |
| Matroska | .mkv | All | All | Best flexibility, open |
| WebM | .webm | VP9, AV1 | Opus | Web-native |
| GIF | .gif | — | — | Animated, no audio |
| Raw H.264 | .h264 | H.264 | — | For advanced users |

### MP4 Fast Start

For web playback, MP4 files use `movflags=+faststart` which relocates the MOOV atom to the file header:

```cpp
av_opt_set(formatContext_->priv_data, "movflags", "+faststart", 0);
```

---

## Export Pipeline

```
[VideoEncoder output packets]
         │
[AudioEncoder output packets]
         │
         ▼
[Muxer] — interleave by DTS
         │
    ┌────┴──────┐
    │           │
[Write Buffer]  [Metadata Injector]
    │           │
    └────┬──────┘
         │
[Output File (disk)]
```

### Muxer

```cpp
class Muxer {
public:
    bool initialize(const std::string& outputPath, 
                    const MuxerConfig& config);
    bool writeVideoPacket(const EncodedPacket& pkt);
    bool writeAudioPacket(const EncodedPacket& pkt);
    bool finalize(); // flush, write index, close file
    
private:
    AVFormatContext*  formatCtx_;
    AVStream*         videoStream_;
    AVStream*         audioStream_;
    int64_t           videoPts_ = 0;
    int64_t           audioPts_ = 0;
};
```

### Metadata Injector

Writes metadata fields into the container at finalization:

```
Title:    "CodeStudio Recording - <date>"
Software: "CodeStudio Recorder v<version>"
Date:     <ISO8601 timestamp>
Duration: <seconds>
Width/Height: <resolution>
```

---

## Encoder Configuration Reference

```cpp
struct VideoEncoderConfig {
    EncoderType  preferred     = EncoderType::Auto;
    VideoCodec   codec         = VideoCodec::H264;
    uint32_t     width;
    uint32_t     height;
    uint32_t     fps           = 60;
    uint32_t     keyframeEvery = 2; // seconds
    
    RateControlMode rcMode     = RateControlMode::VBR;
    uint32_t        bitrate    = 10'000'000; // 10 Mbps
    uint32_t        maxBitrate = 20'000'000; // 20 Mbps
    uint32_t        minBitrate = 4'000'000;  //  4 Mbps
    
    // CRF/CQP quality (lower = better)
    uint32_t        crf        = 18;
    uint32_t        cqp        = 22;
    
    ColorSpace      colorSpace = ColorSpace::BT709;
    PixelFormat     pixFmt     = PixelFormat::NV12;
};
```

---

*Last updated: 2025 | Module 05 of 19*
