#pragma once
#include "i_encoder.h"
#include <string>
#include <vector>

// Forward declarations for FFmpeg
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct SwrContext;

namespace cs {

class FFmpegEncoder : public IEncoder {
public:
    FFmpegEncoder();
    ~FFmpegEncoder();

    bool initialize(const RecordingConfig& config) override;
    void encodeVideoFrame(const VideoFrame& frame) override;
    void encodeAudioBuffer(const AudioBuffer& buffer) override;
    void finalize() override;

    bool generateThumbnail(const std::string& video_path, const std::string& thumb_path) override;

private:
    bool initVideo(const RecordingConfig& config);
    bool initAudio(const RecordingConfig& config);

    AVFormatContext* format_ctx_ = nullptr;

    // Video
    AVCodecContext* video_codec_ctx_ = nullptr;
    AVStream* video_stream_ = nullptr;
    AVFrame* video_frame_ = nullptr;
    SwsContext* sws_ctx_ = nullptr;

    // Audio
    AVCodecContext* audio_codec_ctx_ = nullptr;
    AVStream* audio_stream_ = nullptr;
    AVFrame* audio_frame_ = nullptr;
    SwrContext* swr_ctx_ = nullptr;

    int64_t video_pts_ = 0;
    int64_t audio_pts_ = 0;
};

} // namespace cs
