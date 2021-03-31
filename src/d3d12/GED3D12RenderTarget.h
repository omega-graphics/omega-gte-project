#include "GED3D12.h"
#include "omegaGTE/GERenderTarget.h"
#include "GED3D12CommandQueue.h"

#ifndef OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H
#define OMEGAGRAPHICSENGINE_D3D12_GED3D12RENDERTARGET_H

_NAMESPACE_BEGIN_
    class GED3D12NativeRenderTarget : public GENativeRenderTarget {
        GED3D12Engine *engine;
        ComPtr<IDXGISwapChain3> swapChain;
        SharedHandle<GED3D12CommandQueue> commandQueue;
    public:
         ComPtr<ID3D12DescriptorHeap> descriptorHeapForRenderTarget;
          unsigned frameIndex;
        std::vector<ID3D12Resource *> renderTargets;
        GED3D12NativeRenderTarget(IDXGISwapChain3 * swapChain,
                                 ID3D12DescriptorHeap * descriptorHeapForRenderTarget,
                                 SharedHandle<GED3D12CommandQueue> & commandQueue,
                                 unsigned frameIndex,
                                 ID3D12Resource *const *renderTargetViews,
                                 size_t renderTargetViewCount);
    };

    class GED3D12TextureRenderTarget : public GETextureRenderTarget {
        GED3D12Engine *engine;
        ComPtr<ID3D12Resource> renderTarget;
    public:
        ComPtr<ID3D12Resource> renderTargetView;
        ComPtr<ID3D12DescriptorHeap> descriptorHeapForRenderTarget;
    };
_NAMESPACE_END_

#endif