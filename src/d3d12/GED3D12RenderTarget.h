#include "GED3D12.h"
#include "omegaGTE/GERenderTarget.h"
#include "GED3D12CommandQueue.h"

#ifndef OMEGAGTE_D3D12_GED3D12RENDERTARGET_H
#define OMEGAGTE_D3D12_GED3D12RENDERTARGET_H

_NAMESPACE_BEGIN_
    class GED3D12NativeRenderTarget : public GENativeRenderTarget {
        GED3D12Engine *engine;
        ComPtr<IDXGISwapChain3> swapChain;
        SharedHandle<GECommandQueue> commandQueue;
    public:
        HWND hwnd;
        void *getSwapChain();
        SharedHandle<CommandBuffer> commandBuffer();
        void commitAndPresent();
        void submitCommandBuffer(SharedHandle<CommandBuffer> & commandBuffer);
         ComPtr<ID3D12DescriptorHeap> descriptorHeapForRenderTarget;
          unsigned frameIndex;
        std::vector<ID3D12Resource *> renderTargets;
        GED3D12NativeRenderTarget(IDXGISwapChain3 * swapChain,
                                 ID3D12DescriptorHeap * descriptorHeapForRenderTarget,
                                 SharedHandle<GECommandQueue> commandQueue,
                                 unsigned frameIndex,
                                 ID3D12Resource *const *renderTargets,
                                 size_t renderTargetViewCount,HWND hwnd);
    };

    class GED3D12TextureRenderTarget : public GETextureRenderTarget {
        GED3D12Engine *engine;
        ComPtr<ID3D12Resource> renderTarget;
        SharedHandle<GED3D12CommandQueue> commandQueue;
    public:
        void commit();
        SharedHandle<CommandBuffer> commandBuffer();
        ComPtr<ID3D12Resource> renderTargetView;
        ComPtr<ID3D12DescriptorHeap> descriptorHeapForRenderTarget;
    };
_NAMESPACE_END_

#endif