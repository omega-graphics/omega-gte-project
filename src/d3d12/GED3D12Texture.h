#include "GED3D12.h"
#include "omegaGTE/GETexture.h"

#ifndef OMEGAGTE_D3D12_GED3D12TEXTURE_H
#define OMEGAGTE_D3D12_GED3D12TEXTURE_H

_NAMESPACE_BEGIN_

class GED3D12Texture : public GETexture {
public:
    void copyBytes(void *bytes, size_t len);
    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12DescriptorHeap> descHeap;
    ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
    GED3D12Texture(ID3D12Resource *res,ID3D12DescriptorHeap *descHeap,ID3D12DescriptorHeap *rtvDescHeap);
};

_NAMESPACE_END_

#endif