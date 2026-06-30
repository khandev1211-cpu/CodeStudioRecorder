#include "webcam_utils.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")

namespace cs {

std::vector<WebcamDeviceInfoInternal> WebcamUtils::enumerateCameras() {
    std::vector<WebcamDeviceInfoInternal> cameras;

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) return cameras;

    IMFAttributes* attributes = nullptr;
    hr = MFCreateAttributes(&attributes, 1);
    if (SUCCEEDED(hr)) {
        hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    }

    if (SUCCEEDED(hr)) {
        IMFActivate** devices = nullptr;
        UINT32 count = 0;
        hr = MFEnumDeviceSources(attributes, &devices, &count);

        if (SUCCEEDED(hr)) {
            for (UINT32 i = 0; i < count; i++) {
                LPWSTR name = nullptr;
                UINT32 nameLen = 0;
                devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &nameLen);

                LPWSTR id = nullptr;
                UINT32 idLen = 0;
                devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &id, &idLen);

                cameras.push_back({ id ? id : L"", name ? name : L"Unknown Camera" });

                if (name) CoTaskMemFree(name);
                if (id) CoTaskMemFree(id);
                devices[i]->Release();
            }
            CoTaskMemFree(devices);
        }
    }

    if (attributes) attributes->Release();
    // We don't call MFShutdown here if we plan to use MF elsewhere,
    // but for enumeration it's okay if we balance it.
    // MFShutdown();

    return cameras;
}

} // namespace cs
