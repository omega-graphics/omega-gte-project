#include "GED3D12Texture.h"
#include <cstring>

_NAMESPACE_BEGIN_

GED3D12Texture::GED3D12Texture(const GETextureType & type,
                               const GETextureUsage & usage,
                               const TexturePixelFormat & pixelFormat,
                               ID3D12Resource *res,
                               ID3D12DescriptorHeap *descHeap,
                               ID3D12DescriptorHeap *rtvDescHeap,
                               D3D12_RESOURCE_STATES & currentState):
                                GETexture(type,usage,pixelFormat),
                               resource(res),
                               srvDescHeap(descHeap),
                               rtvDescHeap(rtvDescHeap),
                               currentState(currentState){
    
}

void GED3D12Texture::copyBytes(void *bytes, size_t len){
    void *mem_ptr;
    CD3DX12_RANGE range (0,0);
    resource->Map(0,&range,&mem_ptr);
    std::memcpy(mem_ptr,bytes,len);
    resource->Unmap(0,nullptr);
}

_NAMESPACE_END_