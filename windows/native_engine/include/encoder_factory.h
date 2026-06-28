#pragma once
#include "i_encoder.h"
#include <memory>

namespace cs {

enum class EncoderType {
    Auto,
    Software,
    NVENC,
    QuickSync,
    AMF
};

class EncoderFactory {
public:
    static std::unique_ptr<IEncoder> createEncoder(EncoderType type = EncoderType::Auto);
};

} // namespace cs
