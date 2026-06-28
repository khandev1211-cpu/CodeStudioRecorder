#pragma once
#include "i_audio_engine.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <wrl/client.h>
#include <thread>
#include <atomic>

namespace cs {

class WASAPIAudioEngine : public IAudioEngine {
public:
    enum class DeviceMode {
        Loopback,
        Capture
    };

    WASAPIAudioEngine(DeviceMode mode);
    ~WASAPIAudioEngine();

    bool initialize(const RecordingConfig& config) override;
    void start(AudioCallback callback) override;
    void stop() override;

private:
    void captureLoop();

    DeviceMode mode_;
    Microsoft::WRL::ComPtr<IAudioClient> audio_client_;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> capture_client_;
    HANDLE sample_ready_event_ = NULL;
    WAVEFORMATEX* wave_format_ = nullptr;

    std::thread capture_thread_;
    std::atomic<bool> running_{ false };
    AudioCallback callback_;

    uint32_t buffer_frame_count_ = 0;
};

} // namespace cs
