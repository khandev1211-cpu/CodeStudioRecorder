#pragma once
#include <vector>
#include <string>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>

namespace cs {

struct AudioDeviceInfoInternal {
    std::wstring id;
    std::wstring name;
    bool is_default;
};

class AudioUtils {
public:
    static std::vector<AudioDeviceInfoInternal> enumerateDevices(bool capture);
};

} // namespace cs
