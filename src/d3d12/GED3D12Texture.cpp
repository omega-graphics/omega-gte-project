#include "GED3D12Texture.h"

_NAMESPACE_BEGIN_

GED3D12Texture::GED3D12Texture(ID3D12Resource *res,ID3D12DescriptorHeap *descHeap):resource(res),descHeap(descHeap){
    
};

_NAMESPACE_END_