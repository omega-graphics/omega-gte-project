#include "omegaGE/GE.h"

#include "d3dx12.h"
#include <dxgi1_4.h>
#include <d3d12shader.h>

#include <wrl.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"runtimeobject.lib")

#ifndef OMEGAGRAPHICSENGINE_GED3D12_H
#define OMEGAGRAPHICSENGINE_GED3D12_H

namespace OmegaGE {
    using Microsoft::WRL::ComPtr;

    class GED3D12Engine : public OmegaGraphicsEngine {
        GED3D12Engine();
    public:
        ComPtr<IDXGIFactory4> dxgi_factory;
        ComPtr<ID3D12Device8> d3d12_device;
        ComPtr<ID3D12CommandAllocator> bufferAllocator;
        static SharedHandle<OmegaGraphicsEngine> Create();
    };
}

#endif




