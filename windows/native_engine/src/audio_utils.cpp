#include "audio_utils.h"
#include <iostream>

namespace cs {

std::vector<AudioDeviceInfoInternal> AudioUtils::enumerateDevices(bool capture) {
    std::vector<AudioDeviceInfoInternal> devices;

    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) return devices;

    Microsoft::WRL::ComPtr<IMMDeviceCollection> collection;
    hr = enumerator->EnumAudioEndpoints(capture ? eCapture : eRender, DEVICE_STATE_ACTIVE, &collection);
    if (FAILED(hr)) return devices;

    UINT count;
    collection->GetCount(&count);

    Microsoft::WRL::ComPtr<IMMDevice> defaultDevice;
    std::wstring defaultId = L"";
    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(capture ? eCapture : eRender, eConsole, &defaultDevice))) {
        LPWSTR id;
        defaultDevice->GetId(&id);
        defaultId = id;
        CoTaskMemFree(id);
    }

    for (UINT i = 0; i < count; i++) {
        Microsoft::WRL::ComPtr<IMMDevice> device;
        collection->Item(i, &device);

        LPWSTR id;
        device->GetId(&id);

        Microsoft::WRL::ComPtr<IPropertyStore> props;
        device->OpenPropertyStore(STGM_READ, &props);

        PROPVARIANT varName;
        PropVariantInit(&varName);
        props->GetValue(PKEY_Device_FriendlyName, &varName);

        devices.push_back({ id, varName.pwszVal ? varName.pwszVal : L"Unknown", defaultId == id });

        PropVariantClear(&varName);
        CoTaskMemFree(id);
    }

    return devices;
}

} // namespace cs
