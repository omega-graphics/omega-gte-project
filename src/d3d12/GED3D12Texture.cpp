#include "GED3D12Texture.h"

_NAMESPACE_BEGIN_

GED3D12Texture::GED3D12Texture(ID3D12Resource *res,
                               ID3D12DescriptorHeap *descHeap,
                               ID3D12DescriptorHeap *rtvDescHeap):
                               resource(res),
                               descHeap(descHeap),
                               rtvDescHeap(rtvDescHeap){
    
};

_NAMESPACE_END_