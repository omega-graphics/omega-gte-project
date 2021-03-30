#include "GED3D12RenderTarget.h"

_NAMESPACE_BEGIN_
    GED3D12NativeRenderTarget::GED3D12NativeRenderTarget(HWND hwnd,
        IDXGISwapChain3 * swapChain,
        ID3D12DescriptorHeap * descriptorHeapForRenderTarget,
        SharedHandle<GED3D12CommandQueue> & commandQueue):swapChain(swapChain),descriptorHeapForRenderTarget(descriptorHeapForRenderTarget),commandQueue(commandQueue),hwnd(hwnd){
        
    };
_NAMESPACE_END_