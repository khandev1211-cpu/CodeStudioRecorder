#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <mutex>
#include "frame_queue.h"

namespace cs {

class TexturePool {
public:
    TexturePool(ID3D11Device* device, size_t capacity);
    ~TexturePool() = default;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> acquire(uint32_t width, uint32_t height, DXGI_FORMAT format);
    void release(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture);
    void release(ID3D11Texture2D* texture);

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> pool_;
    std::mutex mutex_;

    // In a production app, we might use a lock-free queue for availability
    // but a simple vector + mutex is fine for initial MVP pool management.
};

} // namespace cs
