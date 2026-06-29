#include "wasapi_audio_engine.h"
#include <iostream>
#include <avrt.h>

#pragma comment(lib, "avrt.lib")

namespace cs {

WASAPIAudioEngine::WASAPIAudioEngine(DeviceMode mode) : mode_(mode) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        // Log error if needed
    }
}

WASAPIAudioEngine::~WASAPIAudioEngine() {
    stop();
    if (wave_format_) CoTaskMemFree(wave_format_);
    CoUninitialize();
}

bool WASAPIAudioEngine::initialize(const RecordingConfig& config) {
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) return false;

    Microsoft::WRL::ComPtr<IMMDevice> device;
    if (mode_ == DeviceMode::Loopback) {
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    } else {
        hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device);
    }
    if (FAILED(hr)) return false;

    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audio_client_);
    if (FAILED(hr)) return false;

    hr = audio_client_->GetMixFormat(&wave_format_);
    if (FAILED(hr)) return false;

    // Initialize based on mode
    DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
    if (mode_ == DeviceMode::Loopback) {
        flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;
    }

    hr = audio_client_->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        flags,
        0, 0, wave_format_, nullptr);
    if (FAILED(hr)) return false;

    hr = audio_client_->GetBufferSize(&buffer_frame_count_);
    if (FAILED(hr)) return false;

    sample_ready_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    hr = audio_client_->SetEventHandle(sample_ready_event_);
    if (FAILED(hr)) return false;

    hr = audio_client_->GetService(__uuidof(IAudioCaptureClient), (void**)&capture_client_);
    if (FAILED(hr)) return false;

    return true;
}

void WASAPIAudioEngine::start(AudioCallback callback) {
    callback_ = callback;
    running_ = true;
    capture_thread_ = std::thread(&WASAPIAudioEngine::captureLoop, this);
    audio_client_->Start();
}

void WASAPIAudioEngine::stop() {
    running_ = false;
    if (capture_thread_.joinable()) {
        capture_thread_.join();
    }
    if (audio_client_) {
        audio_client_->Stop();
    }
    if (sample_ready_event_) {
        CloseHandle(sample_ready_event_);
        sample_ready_event_ = NULL;
    }
}

void WASAPIAudioEngine::captureLoop() {
    DWORD taskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristicsW(L"Pro Audio", &taskIndex);

    while (running_) {
        WaitForSingleObject(sample_ready_event_, INFINITE);

        UINT32 packetLength = 0;
        HRESULT hr = capture_client_->GetNextPacketSize(&packetLength);

        while (packetLength > 0) {
            BYTE* data;
            UINT32 numFramesAvailable;
            DWORD flags;

            hr = capture_client_->GetBuffer(&data, &numFramesAvailable, &flags, nullptr, nullptr);
            if (SUCCEEDED(hr)) {
                if (callback_) {
                    AudioBuffer buffer;
                    buffer.samples = (float*)data; // Assuming float format from mix format
                    buffer.frame_count = numFramesAvailable;
                    buffer.channels = wave_format_->nChannels;
                    buffer.sample_rate = wave_format_->nSamplesPerSec;
                    buffer.timestamp_qpc = 0; // TODO: Get QPC

                    callback_(buffer);
                }
                capture_client_->ReleaseBuffer(numFramesAvailable);
            }

            capture_client_->GetNextPacketSize(&packetLength);
        }
    }

    if (hTask) AvRevertMmThreadCharacteristics(hTask);
}

} // namespace cs
