#pragma once
#include "i_capturer.h"
#include "i_audio_engine.h"
#include "i_encoder.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>

namespace cs {

class MockCapturer : public ICapturer {
public:
    bool initialize(const RecordingConfig& config) override { return true; }
    void start(FrameCallback callback) override {
        running_ = true;
        thread_ = std::thread([this, callback]() {
            while (running_) {
                VideoFrame frame{nullptr, 1920, 1080, 0};
                callback(frame);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        });
    }
    void stop() override {
        running_ = false;
        if (thread_.joinable()) thread_.join();
    }
    void pause() override {}
    void resume() override {}
private:
    std::atomic<bool> running_{false};
    std::thread thread_;
};

class MockAudioEngine : public IAudioEngine {
public:
    bool initialize(const RecordingConfig& config) override { return true; }
    void start(AudioCallback callback) override {
        running_ = true;
        thread_ = std::thread([this, callback]() {
            while (running_) {
                AudioBuffer buf{nullptr, 480, 2, 48000, 0};
                callback(buf);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    void stop() override {
        running_ = false;
        if (thread_.joinable()) thread_.join();
    }
private:
    std::atomic<bool> running_{false};
    std::thread thread_;
};

class MockEncoder : public IEncoder {
public:
    bool initialize(const RecordingConfig& config) override {
        std::cout << "MockEncoder: Writing to " << config.output_path << std::endl;
        return true;
    }
    void encodeVideoFrame(const VideoFrame& frame) override {}
    void encodeAudioBuffer(const AudioBuffer& buffer) override {}
    void finalize() override {
        std::cout << "MockEncoder: Finalizing file" << std::endl;
    }
};

} // namespace cs
