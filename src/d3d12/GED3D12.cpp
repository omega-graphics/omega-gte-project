#include "GED3D12.h"

namespace OmegaGE {
    GED3D12Engine::GED3D12Engine(){
        HRESULT hr;

        hr = D3D12CreateDevice(NULL,D3D_FEATURE_LEVEL_12_1,IID_PPV_ARGS(d3d12_device.GetAddressOf()));
        if(FAILED(hr)){
            exit(1);
        };
        hr = d3d12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(bufferAllocator.GetAddressOf()));
        if(FAILED(hr)){
            exit(1);
        };

    };
};