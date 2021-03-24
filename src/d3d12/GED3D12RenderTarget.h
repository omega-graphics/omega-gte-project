#include "GED3D12.h"
#include "omegaGTE/GERenderTarget.h"
#include "GED3D12CommandQueue.h"

#ifndef OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H
#define OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H

_NAMESPACE_BEGIN_
    class GED3D12NativeRenderTarget : public GENativeRenderTarget {
        GED3D12Engine *engine;
        ComPtr<ID3D12Resource> renderTarget;
        ComPtr<IDXGISwapChain3> swapChain;
        ComPtr<ID3D12DescriptorHeap> descriptorHeapForRenderTarget;
        SharedHandle<GED3D12CommandQueue> commandQueue;
        HWND hwnd;
    public:
        GED3D12NativeRenderTarget(HWND hwnd);
    };

    class GED3D12TextureRenderTarget : public GETextureRenderTarget {
        GED3D12Engine *engine;
        ComPtr<ID3D12Resource> renderTarget;
        ComPtr<ID3D12DescriptorHeap> descriptorHeapForRenderTarget;
    };
_NAMESPACE_END_

#endif