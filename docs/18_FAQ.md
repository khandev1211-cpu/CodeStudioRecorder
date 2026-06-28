# 18 — FAQ

## CodeStudio Recorder — Frequently Asked Questions

---

## General

**Q: Is CodeStudio Recorder free?**
A: Yes. The core recorder is free and open source (MIT license). Future Pro features (advanced AI, cloud sync) may be paid add-ons.

**Q: Does it work on Windows 11?**
A: Yes. CodeStudio Recorder targets Windows 10 20H1 (2004) and later, including Windows 11.

**Q: Does it work on macOS or Linux?**
A: Not currently. The architecture uses Windows-specific APIs (WGC, WASAPI, DXGI, Direct3D). macOS/Linux ports are not planned for Phase 1-3.

**Q: Does recording require an NVIDIA GPU?**
A: No. Hardware encoding supports NVENC (NVIDIA), Quick Sync (Intel iGPU), and AMD AMF. If no hardware encoder is available, software encoding (libx264) is used as fallback.

---

## Recording

**Q: What frame rates are supported?**
A: Up to 144fps, limited by your monitor's refresh rate and the configured output FPS.

**Q: Can I record a specific application window?**
A: Yes. Window capture mode captures a specific application window, including when it's partially off-screen or covered by other windows (via Windows Graphics Capture API on Windows 10 2004+).

**Q: Can I record multiple monitors?**
A: Each recording session captures one source (one window or one monitor). Recording multiple monitors simultaneously is planned for Phase 2.

**Q: Why does my recording have dropped frames?**
A: Common causes: disk write speed too slow (use SSD), CPU overloaded (enable hardware encoding), or the frame queue filled (reduce output FPS).

**Q: Where are recordings saved?**
A: Default: `%USERPROFILE%\Videos\CodeStudio Recordings\`. Configurable in Settings → Recording → Output Directory.

---

## Audio

**Q: Can I record system audio without a microphone?**
A: Yes. Enable "System Audio" and disable "Microphone" in the recording setup.

**Q: My microphone audio sounds echo-y. How do I fix it?**
A: Enable "Echo Cancellation" in Settings → Audio (Phase 2 feature). As a workaround, use headphones to prevent microphone picking up system audio.

**Q: Can I adjust mic and system audio volumes separately?**
A: Yes. The recording setup screen has independent volume sliders for microphone and system audio.

---

## Export

**Q: What formats can I export to?**
A: MP4 (H.264, H.265), MKV, WebM (VP9, AV1). GIF export for short clips (Phase 2).

**Q: How do I create a YouTube Short or Instagram Reel?**
A: Use the Export Presets (Phase 2) — preset options include "YouTube Short (9:16)" and "Instagram Reel (9:16)" which auto-crop and resize.

---

## Known Limitations

| Limitation | Status | Workaround |
|---|---|---|
| Single source per recording | Phase 2 fix | Record separately, combine in video editor |
| No built-in video editor | Phase 2 trim only | Use DaVinci Resolve, CapCut, etc. |
| No macOS / Linux | Not planned | Use OBS on other platforms |
| No streaming | Phase 5 | Use OBS for streaming |
| No 4K hardware encoding on old NVIDIA GPUs | By design | Use software encoding |

---

## Troubleshooting

**Recording won't start:**
- Check that no other app is exclusively capturing the same window (some apps prevent capture)
- Run CodeStudio as Administrator if capturing protected windows
- Check Settings → Capture → Backend and try "DXGI" instead of "WGC"

**No audio in output:**
- Ensure "System Audio" or "Microphone" is enabled in recording setup
- Check Windows audio settings — ensure the correct device is selected
- Verify the audio device is not in exclusive mode by another app

**Installer won't run:**
- Ensure Windows SmartScreen allows the install (signed with EV cert)
- Try right-click → "Run as administrator"

---

*Last updated: 2025 | Module 18 of 19*
