#include "GED3D12RenderTarget.h"

_NAMESPACE_BEGIN_
    GED3D12NativeRenderTarget::GED3D12NativeRenderTarget(
        IDXGISwapChain3 * swapChain,
        ID3D12DescriptorHeap * descriptorHeapForRenderTarget,
        SharedHandle<GED3D12CommandQueue> & commandQueue,unsigned frameIndex,ID3D12Resource *const *renderTargetViews,size_t renderTargetViewCount):swapChain(swapChain),descriptorHeapForRenderTarget(descriptorHeapForRenderTarget),commandQueue(commandQueue),frameIndex(frameIndex),renderTargets(renderTargetViews,renderTargetViews + renderTargetViewCount){
        
    };
_NAMESPACE_END_