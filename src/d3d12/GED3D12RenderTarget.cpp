#include "GED3D12RenderTarget.h"

_NAMESPACE_BEGIN_
    GED3D12NativeRenderTarget::GED3D12NativeRenderTarget(
        IDXGISwapChain3 * swapChain,
        ID3D12DescriptorHeap * descriptorHeapForRenderTarget,
        SharedHandle<GECommandQueue> commandQueue,
        unsigned frameIndex,
        ID3D12Resource *const *renderTargets,
        size_t renderTargetViewCount):swapChain(swapChain),
        descriptorHeapForRenderTarget(descriptorHeapForRenderTarget),
        commandQueue((GED3D12CommandQueue *)commandQueue.get()),frameIndex(frameIndex),
        renderTargets(renderTargets,renderTargets + renderTargetViewCount){
        
    };

    void GED3D12NativeRenderTarget::submitCommandBuffer(SharedHandle<CommandBuffer> & commandBuffer){

    };

    SharedHandle<GERenderTarget::CommandBuffer> GED3D12NativeRenderTarget::commandBuffer(){
        return std::shared_ptr<GERenderTarget::CommandBuffer>(
            new GERenderTarget::CommandBuffer(this,CommandBuffer::GERTType::Native,commandQueue->getAvailableBuffer()));
    };

    void GED3D12NativeRenderTarget::commitAndPresent(){
        HRESULT hr;
        commandQueue->commitToGPU();
        /// NOTE: Maybe a Fence here to prevent gpu timing problems.
        DXGI_PRESENT_PARAMETERS params;
        params.DirtyRectsCount = NULL;
        params.pDirtyRects = NULL;
        params.pScrollOffset = NULL;
        params.pScrollRect = NULL;
        hr = swapChain->Present1(1,0,&params);
        if(FAILED(hr) || hr == DXGI_STATUS_OCCLUDED){
            // MessageBOx
            DEBUG_STREAM("Failed to Present SwapChain");
        }
        else
            frameIndex = swapChain->GetCurrentBackBufferIndex();
    };

    SharedHandle<GERenderTarget::CommandBuffer> GED3D12TextureRenderTarget::commandBuffer(){
        return std::shared_ptr<GERenderTarget::CommandBuffer>(new GERenderTarget::CommandBuffer(this,CommandBuffer::GERTType::Texture,commandQueue->getAvailableBuffer()));
    };

    void GED3D12TextureRenderTarget::commit(){
        HRESULT hr;
        commandQueue->commitToGPU();
        /// TODO: Fence.
    };

_NAMESPACE_END_