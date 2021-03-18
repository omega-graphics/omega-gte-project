#include "GED3D12.h"
#include "omegaGE/GERenderTarget.h"

#ifndef OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H
#define OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H

namespace OmegaGE {
    class GED3D12NativeRenderTarget : public GENativeRenderTarget {
        GED3D12Engine *engine;
        ComPtr<IDXGISwapChain3> swapChain;
        HWND hwnd;
    public:

    };

    class GED3D12TextureRenderTarget : public GETextureRenderTarget {
        
    };
};

#endif