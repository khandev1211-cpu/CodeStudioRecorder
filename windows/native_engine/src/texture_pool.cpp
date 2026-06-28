#include "texture_pool.h"

namespace cs {

TexturePool::TexturePool(ID3D11Device* device, size_t capacity) : device_(device) {
    pool_.reserve(capacity);
}

Microsoft::WRL::ComPtr<ID3D11Texture2D> TexturePool::acquire(uint32_t width, uint32_t height, DXGI_FORMAT format) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = pool_.begin(); it != pool_.end(); ++it) {
        D3D11_TEXTURE2D_DESC desc;
        (*it)->GetDesc(&desc);

        if (desc.Width == width && desc.Height == height && desc.Format == format) {
            auto tex = *it;
            pool_.erase(it);
            return tex;
        }
    }

    // If no suitable texture found, create a new one
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &texture);
    if (FAILED(hr)) return nullptr;

    return texture;
}

void TexturePool::release(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture) {
    if (!texture) return;
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.push_back(texture);
}

} // namespace cs
