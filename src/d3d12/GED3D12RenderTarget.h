#include "GED3D12.h"
#include "omegaGE/GERenderTarget.h"

#ifndef OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H
#define OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H

namespace OmegaGE {
    class GED3D12NativeRenderTarget : public GENativeRenderTarget {
        GED3D12Engine *engine;
        ComPtr<ID3D12Resource> renderTarget;
        ComPtr<IDXGISwapChain3> swapChain;
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;
        HWND hwnd;
    public:
        GED3D12NativeRenderTarget(HWND hwnd);
    };

    class GED3D12TextureRenderTarget : public GETextureRenderTarget {
        GED3D12Engine *engine;
        ComPtr<ID3D12Resource> renderTarget;
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    };
};

#endif