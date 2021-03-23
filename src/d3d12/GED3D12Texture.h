#include "GED3D12.h"
#include "omegaGTE/GETexture.h"

#ifndef OMEGAGTE_D3D12_GED3D12TEXTURE_H
#define OMEGAGTE_D3D12_GED3D12TEXTURE_H

_NAMESPACE_BEGIN_

class GED3D12Texture : public GETexture {
    ComPtr<ID3D12Resource> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
public:
    GED3D12Texture(ID3D12Resource *res);
};

_NAMESPACE_END_

#endif