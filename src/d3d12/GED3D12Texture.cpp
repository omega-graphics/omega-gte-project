#include "GED3D12Texture.h"
#include <cstring>

_NAMESPACE_BEGIN_

GED3D12Texture::GED3D12Texture(ID3D12Resource *res,
                               ID3D12DescriptorHeap *descHeap,
                               ID3D12DescriptorHeap *rtvDescHeap):
                               resource(res),
                               srvDescHeap(descHeap),
                               rtvDescHeap(rtvDescHeap){
    
}

void GED3D12Texture::copyBytes(void *bytes, size_t len){
    void *mem_ptr;
    CD3DX12_RANGE range (0,0);
    resource->Map(0,&range,&mem_ptr);
    std::memcpy(mem_ptr,bytes,len);
    resource->Unmap(0,nullptr);
}

_NAMESPACE_END_