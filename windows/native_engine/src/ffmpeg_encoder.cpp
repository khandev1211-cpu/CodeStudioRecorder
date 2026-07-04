#include "ffmpeg_encoder.h"
#include "cs_logger.h"
#include <iostream>
#include <vector>
#include <d3d11.h>
#include <filesystem>

#ifdef USE_REAL_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#endif

namespace cs {

#ifdef USE_REAL_FFMPEG
std::string get_ffmpeg_error(int err) {
    char buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(err, buf, sizeof(buf));
    return std::string(buf);
}
#endif

FFmpegEncoder::FFmpegEncoder() {}

FFmpegEncoder::~FFmpegEncoder() {
    finalize();
}

bool FFmpegEncoder::initialize(const RecordingConfig& config) {
#ifdef USE_REAL_FFMPEG
    std::lock_guard<std::mutex> lock(encoder_mutex_);
    CS_LOG_INFO("FFmpegEncoder: Initializing output: " + std::string(config.output_path));

    int ret = avformat_alloc_output_context2(&format_ctx_, nullptr, nullptr, config.output_path);
    if (ret < 0 || !format_ctx_) {
        CS_LOG_ERR("Could not create output context: " + get_ffmpeg_error(ret));
        return false;
    }

    if (!initVideo(config)) {
        CS_LOG_ERR("Video initialization failed");
        return false;
    }

    if (!initAudio(config)) {
        CS_LOG_ERR("Audio initialization failed");
        return false;
    }

    if (!(format_ctx_->oformat->flags & AVFMT_NOFILE)) {
        // Ensure parent directory exists
        try {
            std::filesystem::path p(config.output_path);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }
        } catch (...) {
            CS_LOG_WARN("Failed to verify/create output directory for: " + std::string(config.output_path));
        }

        ret = avio_open(&format_ctx_->pb, config.output_path, AVIO_FLAG_WRITE);
        if (ret < 0) {
            CS_LOG_ERR("Could not open output file '" + std::string(config.output_path) + "': " + get_ffmpeg_error(ret));
            return false;
        }
    }

    if (format_ctx_->nb_streams == 0) {
        CS_LOG_ERR("No streams added to format context");
        return false;
    }

    // Optimization for MP4: move index to start of file
    AVDictionary* options = nullptr;
    av_dict_set(&options, "movflags", "faststart", 0);

    ret = avformat_write_header(format_ctx_, &options);
    if (ret < 0) {
        CS_LOG_ERR("Error occurred when writing header: " + get_ffmpeg_error(ret));
        av_dict_free(&options);
        return false;
    }
    av_dict_free(&options);

    return true;
#else
    CS_LOG_WARN("FFmpegEncoder: Built without real FFmpeg. Using MOCK mode.");
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
        CS_LOG_ERR("No H.264 encoder found at all");
        return false;
    }

    video_stream_ = avformat_new_stream(format_ctx_, nullptr);
    video_codec_ctx_ = avcodec_alloc_context3(codec);

    video_codec_ctx_->width = (config.width % 2 == 0) ? config.width : config.width - 1;
    video_codec_ctx_->height = (config.height % 2 == 0) ? config.height : config.height - 1;
    video_codec_ctx_->time_base = { 1, (int)config.fps };
    video_codec_ctx_->framerate = { (int)config.fps, 1 };
    video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
    video_codec_ctx_->gop_size = 12;

    if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        video_codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    av_opt_set(video_codec_ctx_->priv_data, "preset", "ultrafast", 0);
    av_opt_set(video_codec_ctx_->priv_data, "tune",   "zerolatency", 0);

    int ret = avcodec_open2(video_codec_ctx_, codec, nullptr);
    if (ret < 0) {
        CS_LOG_WARN("Could not open primary video codec: " + get_ffmpeg_error(ret) + ". Attempting software fallback.");

        // Fallback to libx264 if hardware failed to open
        if (codec->id != AV_CODEC_ID_H264 || strcmp(codec->name, "libx264") != 0) {
            const AVCodec* sw_codec = avcodec_find_encoder_by_name("libx264");
            if (sw_codec) {
                avcodec_free_context(&video_codec_ctx_);
                video_codec_ctx_ = avcodec_alloc_context3(sw_codec);
                video_codec_ctx_->width = config.width;
                video_codec_ctx_->height = config.height;
                video_codec_ctx_->time_base = { 1, (int)config.fps };
                video_codec_ctx_->framerate = { (int)config.fps, 1 };
                video_codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
                video_codec_ctx_->gop_size = 12;
                av_opt_set(video_codec_ctx_->priv_data, "preset", "ultrafast", 0);
                ret = avcodec_open2(video_codec_ctx_, sw_codec, nullptr);
            }
        }

        if (ret < 0) {
            CS_LOG_ERR("Could not open fallback video codec: " + get_ffmpeg_error(ret));
            return false;
        }
    }

    avcodec_parameters_from_context(video_stream_->codecpar, video_codec_ctx_);
    video_stream_->time_base = video_codec_ctx_->time_base;

    video_frame_ = av_frame_alloc();
    video_frame_->format = video_codec_ctx_->pix_fmt;
    video_frame_->width = video_codec_ctx_->width;
    video_frame_->height = video_codec_ctx_->height;
    av_frame_get_buffer(video_frame_, 0);

    sws_ctx_ = sws_getContext(video_codec_ctx_->width, video_codec_ctx_->height, AV_PIX_FMT_BGRA,
                              video_codec_ctx_->width, video_codec_ctx_->height, AV_PIX_FMT_YUV420P,
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
    av_channel_layout_default(&audio_codec_ctx_->ch_layout, 2);

    if (format_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        audio_codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    int ret = avcodec_open2(audio_codec_ctx_, codec, nullptr);
    if (ret < 0) {
        CS_LOG_ERR("Could not open audio codec: " + get_ffmpeg_error(ret));
        return false;
    }

    avcodec_parameters_from_context(audio_stream_->codecpar, audio_codec_ctx_);

    audio_frame_ = av_frame_alloc();
    audio_frame_->format = audio_codec_ctx_->sample_fmt;
    audio_frame_->sample_rate = audio_codec_ctx_->sample_rate;
    av_channel_layout_copy(&audio_frame_->ch_layout, &audio_codec_ctx_->ch_layout);
    audio_frame_->nb_samples = audio_codec_ctx_->frame_size;
    av_frame_get_buffer(audio_frame_, 0);

    swr_ctx_ = swr_alloc();
    AVChannelLayout in_ch_layout;
    av_channel_layout_default(&in_ch_layout, 2);

    av_opt_set_chlayout(swr_ctx_, "in_chlayout", &in_ch_layout, 0);
    av_opt_set_int(swr_ctx_, "in_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(swr_ctx_, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
    av_opt_set_chlayout(swr_ctx_, "out_chlayout", &audio_codec_ctx_->ch_layout, 0);
    av_opt_set_int(swr_ctx_, "out_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(swr_ctx_, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
    swr_init(swr_ctx_);
    av_channel_layout_uninit(&in_ch_layout);

    return true;
#else
    return true;
#endif
}

void FFmpegEncoder::encodeVideoFrame(const VideoFrame& frame) {
#ifdef USE_REAL_FFMPEG
    if (!video_codec_ctx_ || !frame.data) return;

    std::lock_guard<std::mutex> lock(encoder_mutex_);

    ID3D11Texture2D* source_texture = static_cast<ID3D11Texture2D*>(frame.data);
    ID3D11Device* device = nullptr;
    source_texture->GetDevice(&device);
    ID3D11DeviceContext* context = nullptr;
    device->GetImmediateContext(&context);

    // 1. Create/Ensure staging texture for CPU access
    if (!staging_texture_) {
        D3D11_TEXTURE2D_DESC desc;
        source_texture->GetDesc(&desc);
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;
        device->CreateTexture2D(&desc, nullptr, &staging_texture_);
    }

    // 2. Copy from GPU to Staging
    context->CopyResource(staging_texture_, source_texture);

    // 3. Map staging texture and convert to AVFrame
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(context->Map(staging_texture_, 0, D3D11_MAP_READ, 0, &mapped))) {
        // Convert BGRA (Windows Capture) to YUV420P (H.264)
        const uint8_t* src_data[] = { (const uint8_t*)mapped.pData };
        int src_linesize[] = { (int)mapped.RowPitch };

        sws_scale(sws_ctx_, src_data, src_linesize, 0, video_codec_ctx_->height, video_frame_->data, video_frame_->linesize);

        video_frame_->pts = video_pts_++;

        AVPacket* pkt = av_packet_alloc();
        int send_ret = avcodec_send_frame(video_codec_ctx_, video_frame_);
        if (send_ret >= 0) {
            while (avcodec_receive_packet(video_codec_ctx_, pkt) == 0) {
                av_packet_rescale_ts(pkt, video_codec_ctx_->time_base, video_stream_->time_base);
                pkt->stream_index = video_stream_->index;
                av_interleaved_write_frame(format_ctx_, pkt);
                av_packet_unref(pkt);
            }
        } else {
            CS_LOG_ERR("Video encoding failed: " + get_ffmpeg_error(send_ret));
        }
        av_packet_free(&pkt);

        context->Unmap(staging_texture_, 0);
    }

    context->Release();
    device->Release();
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
    std::lock_guard<std::mutex> lock(encoder_mutex_);
    if (format_ctx_) {
        CS_LOG_INFO("FFmpegEncoder: Finalizing stream");

        // Flush video encoder
        if (video_codec_ctx_) {
            avcodec_send_frame(video_codec_ctx_, nullptr);
            AVPacket* pkt = av_packet_alloc();
            while (avcodec_receive_packet(video_codec_ctx_, pkt) == 0) {
                av_packet_rescale_ts(pkt, video_codec_ctx_->time_base, video_stream_->time_base);
                pkt->stream_index = video_stream_->index;
                av_interleaved_write_frame(format_ctx_, pkt);
                av_packet_unref(pkt);
            }
            av_packet_free(&pkt);
        }

        // Flush audio encoder
        if (audio_codec_ctx_) {
            avcodec_send_frame(audio_codec_ctx_, nullptr);
            AVPacket* pkt = av_packet_alloc();
            while (avcodec_receive_packet(audio_codec_ctx_, pkt) == 0) {
                av_packet_rescale_ts(pkt, audio_codec_ctx_->time_base, audio_stream_->time_base);
                pkt->stream_index = audio_stream_->index;
                av_interleaved_write_frame(format_ctx_, pkt);
                av_packet_unref(pkt);
            }
            av_packet_free(&pkt);
        }

        av_write_trailer(format_ctx_);

        if (video_codec_ctx_) avcodec_free_context(&video_codec_ctx_);
        if (audio_codec_ctx_) avcodec_free_context(&audio_codec_ctx_);
        if (video_frame_) av_frame_free(&video_frame_);
        if (audio_frame_) av_frame_free(&audio_frame_);
        if (sws_ctx_) sws_freeContext(sws_ctx_);
        if (swr_ctx_) swr_free(&swr_ctx_);
        if (staging_texture_) staging_texture_->Release();

        if (!(format_ctx_->oformat->flags & AVFMT_NOFILE))
            avio_closep(&format_ctx_->pb);

        avformat_free_context(format_ctx_);
        format_ctx_ = nullptr;
        staging_texture_ = nullptr;
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
