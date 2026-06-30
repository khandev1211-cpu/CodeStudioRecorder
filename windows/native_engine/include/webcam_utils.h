#pragma once
#include <vector>
#include <string>

namespace cs {

struct WebcamDeviceInfoInternal {
    std::wstring id;
    std::wstring name;
};

class WebcamUtils {
public:
    static std::vector<WebcamDeviceInfoInternal> enumerateCameras();
};

} // namespace cs
