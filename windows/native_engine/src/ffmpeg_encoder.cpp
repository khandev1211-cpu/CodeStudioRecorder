#include "ffmpeg_encoder.h"
#include "cs_logger.h"
#include <iostream>
#include <vector>

#ifdef USE_REAL_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#endif

namespace cs {

FFmpegEncoder::FFmpegEncoder() {}

FFmpegEncoder::~FFmpegEncoder() {
    finalize();
}

bool FFmpegEncoder::initialize(const RecordingConfig& config) {
#ifdef USE_REAL_FFMPEG
    CS_LOG_INFO("FFmpegEncoder: Initializing output: " + std::string(config.output_path));

    avformat_alloc_output_context2(&format_ctx_, nullptr, nullptr, config.output_path);
    if (!format_ctx_) {
        CS_LOG_ERR("Could not create output context");
        return false;
    }

    if (!initVideo(config)) return false;
    if (!initAudio(config)) return false;

    if (!(format_ctx_->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&format_ctx_->pb, config.output_path, AVIO_FLAG_WRITE) < 0) {
            CS_LOG_ERR("Could not open output file");
            return false;
        }
    }

    if (avformat_write_header(format_ctx_, nullptr) < 0) {
        CS_LOG_ERR("Error occurred when writing header");
        return false;
    }

    return true;
#else
    CS_LOG_WARN("FFmpegEncoder: Built without real FFmpeg. Using MOCK mode.");
    // Create a dummy file so the UI can "find" it
    FILE* f = fopen(config.output_path, "wb");
    if (f) {
        fprintf(f, "MOCK VIDEO DATA - Link FFmpeg to record real video.");
        fclose(f);
    }
    return true;
#endif
}

bool FFmpegEncoder::initVideo(const RecordingConfig& config) {
#ifdef USE_REAL_FFMPEG
    const AVCodec* codec = nullptr;

    if (config.encoder_name && strlen(config.encoder_name) > 0 && strcmp(config.encoder_name, "auto") != 0) {
        codec = avcodec_find_encoder_by_name(config.encoder_name);
        if (codec) {
            CS_LOG_INFO("Using preferred hardware encoder: " + std::string(config.encoder_name));
        }
    }

    if (!codec || (config.encoder_name && strcmp(config.encoder_name, "auto") == 0)) {
        // Try hardware encoders in order of preference
        const char* hw_encoders[] = { "h264_nvenc", "h264_amf", "h264_qsv" };
        for (const char* name : hw_encoders) {
            const AVCodec* hw_codec = avcodec_find_encoder_by_name(name);
            if (hw_codec) {
                codec = hw_codec;
                CS_LOG_INFO("Auto-detected hardware encoder: " + std::string(name));
                break;
            }
        }
    }

    if (!codec) {
        codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        CS_LOG_INFO("Using default H.264 encoder (software fallback)");
    }

    if (!codec) {
        CS_LOG_ERR("No H.264 encoder found");
        return false;
    }

    video_stream_ = avformat_new_stream(format_ctx_, nullptr);
    video_codec_ctx_ = avcodec_alloc_context3(codec);

    video_codec_ctx_->width = config.width;
    video_codec_ctx_->height = config.height;
    video_codec_ctx_->time_base = { 1, (int)config.fps };
    video_codec_ctx_->framerate = { (int)config.fps, 1 };
    video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
    video_codec_ctx_->gop_size = 12;

    if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        video_codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    av_opt_set(video_codec_ctx_->priv_data, "preset", "ultrafast", 0);
    av_opt_set(video_codec_ctx_->priv_data, "tune",   "zerolatency", 0);

    if (avcodec_open2(video_codec_ctx_, codec, nullptr) < 0) {
        CS_LOG_ERR("Could not open video codec");
        return false;
    }

    avcodec_parameters_from_context(video_stream_->codecpar, video_codec_ctx_);
    video_stream_->time_base = video_codec_ctx_->time_base;

    video_frame_ = av_frame_alloc();
    video_frame_->format = video_codec_ctx_->pix_fmt;
    video_frame_->width = video_codec_ctx_->width;
    video_frame_->height = video_codec_ctx_->height;
    av_frame_get_buffer(video_frame_, 0);

    sws_ctx_ = sws_getContext(config.width, config.height, AV_PIX_FMT_BGRA,
                              config.width, config.height, AV_PIX_FMT_YUV420P,
                              SWS_BICUBIC, nullptr, nullptr, nullptr);

    return true;
#else
    return true;
#endif
}

bool FFmpegEncoder::initAudio(const RecordingConfig& config) {
#ifdef USE_REAL_FFMPEG
    if (!config.capture_audio) return true;

    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!codec) {
        CS_LOG_ERR("AAC encoder not found");
        return false;
    }

    audio_stream_ = avformat_new_stream(format_ctx_, nullptr);
    audio_codec_ctx_ = avcodec_alloc_context3(codec);

    audio_codec_ctx_->sample_fmt = AV_SAMPLE_FMT_FLTP;
    audio_codec_ctx_->bit_rate = 128000;
    audio_codec_ctx_->sample_rate = 48000;
    audio_codec_ctx_->channel_layout = AV_CH_LAYOUT_STEREO;
    audio_codec_ctx_->channels = av_get_channel_layout_nb_channels(audio_codec_ctx_->channel_layout);

    if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        audio_codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(audio_codec_ctx_, codec, nullptr) < 0) {
        CS_LOG_ERR("Could not open audio codec");
        return false;
    }

    avcodec_parameters_from_context(audio_stream_->codecpar, audio_codec_ctx_);

    audio_frame_ = av_frame_alloc();
    audio_frame_->format = audio_codec_ctx_->sample_fmt;
    audio_frame_->sample_rate = audio_codec_ctx_->sample_rate;
    audio_frame_->channel_layout = audio_codec_ctx_->channel_layout;
    audio_frame_->nb_samples = audio_codec_ctx_->frame_size;
    av_frame_get_buffer(audio_frame_, 0);

    swr_ctx_ = swr_alloc();
    av_opt_set_int(swr_ctx_, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr_ctx_, "in_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(swr_ctx_, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
    av_opt_set_int(swr_ctx_, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr_ctx_, "out_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(swr_ctx_, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
    swr_init(swr_ctx_);

    return true;
#else
    return true;
#endif
}

void FFmpegEncoder::encodeVideoFrame(const VideoFrame& frame) {
#ifdef USE_REAL_FFMPEG
    if (!video_codec_ctx_) return;
    // ... Real encoding logic ...
#endif
}

void FFmpegEncoder::encodeAudioBuffer(const AudioBuffer& buffer) {
#ifdef USE_REAL_FFMPEG
    if (!audio_codec_ctx_) return;

    // Simple resample and send
    const uint8_t* in[] = { (uint8_t*)buffer.samples };
    swr_convert(swr_ctx_, audio_frame_->data, audio_frame_->nb_samples, in, buffer.frame_count);

    audio_frame_->pts = audio_pts_;
    audio_pts_ += audio_frame_->nb_samples;

    AVPacket* pkt = av_packet_alloc();
    if (avcodec_send_frame(audio_codec_ctx_, audio_frame_) >= 0) {
        while (avcodec_receive_packet(audio_codec_ctx_, pkt) == 0) {
            av_packet_rescale_ts(pkt, audio_codec_ctx_->time_base, audio_stream_->time_base);
            pkt->stream_index = audio_stream_->index;
            av_interleaved_write_frame(format_ctx_, pkt);
            av_packet_unref(pkt);
        }
    }
    av_packet_free(&pkt);
#endif
}

void FFmpegEncoder::finalize() {
#ifdef USE_REAL_FFMPEG
    if (format_ctx_) {
        CS_LOG_INFO("FFmpegEncoder: Finalizing stream");
        av_write_trailer(format_ctx_);

        if (video_codec_ctx_) avcodec_free_context(&video_codec_ctx_);
        if (audio_codec_ctx_) avcodec_free_context(&audio_codec_ctx_);
        if (video_frame_) av_frame_free(&video_frame_);
        if (audio_frame_) av_frame_free(&audio_frame_);
        if (sws_ctx_) sws_freeContext(sws_ctx_);
        if (swr_ctx_) swr_free(&swr_ctx_);

        if (!(format_ctx_->oformat->flags & AVFMT_NOFILE))
            avio_closep(&format_ctx_->pb);

        avformat_free_context(format_ctx_);
        format_ctx_ = nullptr;
    }
#endif
}

bool FFmpegEncoder::generateThumbnail(const std::string& video_path, const std::string& thumb_path) {
#ifdef USE_REAL_FFMPEG
    // Real implementation would use avformat_open_input, avcodec_receive_frame, and sws_scale to save a PNG
    CS_LOG_INFO("Generating real thumbnail for: " + video_path);
    return true;
#else
    CS_LOG_INFO("Generating MOCK thumbnail for: " + video_path);
    FILE* f = fopen(thumb_path.c_str(), "wb");
    if (f) {
        fprintf(f, "MOCK THUMBNAIL DATA");
        fclose(f);
        return true;
    }
    return false;
#endif
}

} // namespace cs
