#include "GED3D12.h"
#include "omegaGTE/GETexture.h"

#ifndef OMEGAGTE_D3D12_GED3D12TEXTURE_H
#define OMEGAGTE_D3D12_GED3D12TEXTURE_H

_NAMESPACE_BEGIN_

class GED3D12Texture : public GETexture {
public:
    void copyBytes(void *bytes, size_t len) override;

    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12DescriptorHeap> srvDescHeap;
    ComPtr<ID3D12DescriptorHeap> rtvDescHeap;

    D3D12_RESOURCE_STATES currentState;

    explicit GED3D12Texture(
            const GETextureType & type,
            const GETextureUsage & usage,
            const TexturePixelFormat & pixelFormat,
            ID3D12Resource *res,
            ID3D12DescriptorHeap *descHeap,
            ID3D12DescriptorHeap *rtvDescHeap,
            D3D12_RESOURCE_STATES & currentState);
};

_NAMESPACE_END_

#endif