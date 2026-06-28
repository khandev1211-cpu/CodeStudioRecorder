#include "encoder_factory.h"
#include "ffmpeg_encoder.h"
#include "mock_engines.h"

namespace cs {

std::unique_ptr<IEncoder> EncoderFactory::createEncoder(EncoderType type) {
    // For MVP, we use the FFmpegEncoder which will be our main implementation.
    // In a full implementation, we'd probe hardware here.
    return std::make_unique<FFmpegEncoder>();
}

} // namespace cs
