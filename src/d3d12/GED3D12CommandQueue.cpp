#include "GED3D12CommandQueue.h"
#include "GED3D12RenderTarget.h"
#include "GED3D12Pipeline.h"
#include "GED3D12Texture.h"
_NAMESPACE_BEGIN_
    // GED3D12CommandBuffer::GED3D12CommandBuffer(){};
    // void GED3D12CommandBuffer::commitToBuffer(){};
    GED3D12CommandQueue::GED3D12CommandQueue(GED3D12Engine *engine,unsigned size):GECommandQueue(size),engine(engine),currentCount(0){
        HRESULT hr;
        hr = engine->d3d12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(bufferAllocator.GetAddressOf()));

        if(FAILED(hr)){
            exit(1);
        };

        D3D12_COMMAND_QUEUE_DESC desc;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = engine->d3d12_device->GetNodeCount();
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = engine->d3d12_device->CreateCommandQueue(&desc,IID_PPV_ARGS(&commandQueue));
        if(FAILED(hr)){
            MessageBoxA(GetForegroundWindow(),"Failed to Create Command Queue.","NOTE",MB_OK);
            exit(1);
        };

    };

    GED3D12CommandBuffer::GED3D12CommandBuffer(ID3D12GraphicsCommandList6 *commandList,GED3D12CommandQueue *parentQueue):commandList(commandList),parentQueue(parentQueue),inComputePass(false),inBlitPass(false){
        
    };

    void GED3D12CommandBuffer::startBlitPass(){
        inBlitPass = true;
    };

    void GED3D12CommandBuffer::finishBlitPass(){
        inBlitPass = false;
    };

    void GED3D12CommandBuffer::startRenderPass(const GERenderPassDescriptor &desc){
        assert(!inComputePass && "Cannot start a Render Pass while in a compute pass.");
        D3D12_RENDER_PASS_RENDER_TARGET_DESC rt_desc;
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;

        if(desc.nRenderTarget) {
            GED3D12NativeRenderTarget *nativeRenderTarget = (GED3D12NativeRenderTarget *)desc.nRenderTarget;
            cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(nativeRenderTarget->descriptorHeapForRenderTarget->GetCPUDescriptorHandleForHeapStart());
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(nativeRenderTarget->renderTargets[nativeRenderTarget->frameIndex],D3D12_RESOURCE_STATE_PRESENT,D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList->ResourceBarrier(1,&barrier);
        }
        else if(desc.tRenderTarget){
            GED3D12TextureRenderTarget *textureRenderTarget = (GED3D12TextureRenderTarget *)desc.tRenderTarget;
            cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(textureRenderTarget->descriptorHeapForRenderTarget->GetCPUDescriptorHandleForHeapStart());
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(textureRenderTarget->renderTargetView.Get(),D3D12_RESOURCE_STATE_PRESENT,D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList->ResourceBarrier(1,&barrier);
        };
        rt_desc.cpuDescriptor = cpu_handle;
        switch (desc.colorAttachment->loadAction) {
            case GERenderPassDescriptor::ColorAttachment::Load : {
                rt_desc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                rt_desc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                break;
            }
            case GERenderPassDescriptor::ColorAttachment::LoadPreserve : {
                rt_desc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                rt_desc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                break;
            }
            case GERenderPassDescriptor::ColorAttachment::Discard : {
                rt_desc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                rt_desc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                break;
            }
            case GERenderPassDescriptor::ColorAttachment::Clear : {
                rt_desc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                std::initializer_list<FLOAT> colors = {desc.colorAttachment->clearColor.r,desc.colorAttachment->clearColor.g,desc.colorAttachment->clearColor.b,desc.colorAttachment->clearColor.a};
                rt_desc.BeginningAccess.Clear.ClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM,colors.begin());
                /// Same as StoreAction in Metal
                rt_desc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                break;
            }
        }
        
        commandList->BeginRenderPass(1,&rt_desc,nullptr,D3D12_RENDER_PASS_FLAG_NONE);
    };

    void GED3D12CommandBuffer::setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState){
         assert(!inComputePass && "Cannot set Render Pipeline State while in Compute Pass");
        GED3D12RenderPipelineState *d3d12_pipeline_state = (GED3D12RenderPipelineState *)pipelineState.get();
        commandList->SetPipelineState(d3d12_pipeline_state->pipelineState.Get());
    };

    void GED3D12CommandBuffer::setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned int index){
        assert((!inComputePass && !inBlitPass) && "Cannot set Resource Const at a Vertex Func when not in render pass");
        GED3D12Buffer *d3d12_buffer = (GED3D12Buffer *)buffer.get();
        commandList->SetGraphicsRootShaderResourceView(index,d3d12_buffer->buffer->GetGPUVirtualAddress());
    };

    void GED3D12CommandBuffer::setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned int index){
         assert((!inComputePass && !inBlitPass) &&"Cannot set Resource Const at a Vertex Func when not in render pass");
        GED3D12Texture *d3d12_texture = (GED3D12Texture *)texture.get();
        commandList->SetGraphicsRootDescriptorTable(index,d3d12_texture->descHeap->GetGPUDescriptorHandleForHeapStart());
        descriptorHeapBuffer.push_back(d3d12_texture->descHeap.Get());
    };

    void GED3D12CommandBuffer::setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned int index){
         assert((!inComputePass && !inBlitPass) && "Cannot set Resource Const a Fragment Func when not in render pass");
        GED3D12Buffer *d3d12_buffer = (GED3D12Buffer *)buffer.get();
        commandList->SetGraphicsRootShaderResourceView(index,d3d12_buffer->buffer->GetGPUVirtualAddress());
    };

    void GED3D12CommandBuffer::setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned int index){
         assert((!inComputePass && !inBlitPass) && "Cannot set Resource Const a Fragment Func when not in render pass");
         GED3D12Texture *d3d12_texture = (GED3D12Texture *)texture.get();
         commandList->SetGraphicsRootDescriptorTable(index,d3d12_texture->descHeap->GetGPUDescriptorHandleForHeapStart());
         descriptorHeapBuffer.push_back(d3d12_texture->descHeap.Get());
    };

    void GED3D12CommandBuffer::setViewports(std::vector<GEViewport> viewports){
        std::vector<D3D12_VIEWPORT> d3d12_viewports;
        auto viewports_it = viewports.begin();
        while(viewports_it != viewports.end()){
            GEViewport & viewport = *viewports_it;
            CD3DX12_VIEWPORT v(viewport.x,viewport.height - viewport.y,viewport.width,viewport.height,viewport.nearDepth,viewport.farDepth);
            d3d12_viewports.push_back(std::move(v));
            ++viewports_it;
        };
        commandList->RSSetViewports(d3d12_viewports.size(),d3d12_viewports.data());
    };

    void GED3D12CommandBuffer::setScissorRects(std::vector<GEScissorRect> scissorRects){
        std::vector<D3D12_RECT> d3d12_rects;
        auto rects_it = scissorRects.begin();
        while(rects_it != scissorRects.end()){
            GEScissorRect & rect = *rects_it;
            CD3DX12_RECT r(rect.x,rect.y,rect.width + rect.x,rect.height + rect.y);
            d3d12_rects.push_back(std::move(r));
            ++rects_it;
        };
        commandList->RSSetScissorRects(d3d12_rects.size(),d3d12_rects.data());
    };

    void GED3D12CommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx){
        assert(!inComputePass && "Cannot Draw Polygons while in Compute Pass");
        D3D12_PRIMITIVE_TOPOLOGY topology;
        if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::Triangle){
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
        else if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::TriangleStrip){
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        };
        commandList->IASetPrimitiveTopology(topology);
        commandList->DrawInstanced(1,vertexCount,startIdx,0);
    };

    void GED3D12CommandBuffer::finishRenderPass(){
        commandList->SetDescriptorHeaps(descriptorHeapBuffer.size(),descriptorHeapBuffer.data());
        commandList->EndRenderPass();
        descriptorHeapBuffer.clear();
    };

    void GED3D12CommandBuffer::startComputePass(const GEComputePassDescriptor &desc){
        inComputePass = true;
    };

    void GED3D12CommandBuffer::setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState){
        GED3D12ComputePipelineState *d3d12_pipeline_state = (GED3D12ComputePipelineState *)pipelineState.get();
        commandList->SetPipelineState(d3d12_pipeline_state->pipelineState.Get());
    };

    void GED3D12CommandBuffer::finishComputePass(){
        inComputePass = false;
    };

    // void GED3D12CommandBuffer::commitToQueue(){
    //     HRESULT hr;
    //     hr = commandList->Close();
        
    //     parentQueue->commandLists.push_back(commandList.Get());
    // };

    void GED3D12CommandQueue::submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer){
        HRESULT hr;
        auto d3d12_buffer = (GED3D12CommandBuffer *)commandBuffer.get();
        hr = d3d12_buffer->commandList->Close();
        
        commandLists.push_back(d3d12_buffer->commandList.Get());
    };

    void GED3D12CommandBuffer::reset(){
        commandList->Reset(parentQueue->bufferAllocator.Get(),nullptr);
    };

    void GED3D12CommandQueue::commitToGPU(){
        commandQueue->ExecuteCommandLists(commandLists.size(),(ID3D12CommandList *const *)commandLists.data());
    };

    SharedHandle<GECommandBuffer> GED3D12CommandQueue::getAvailableBuffer(){
        HRESULT hr;
        ID3D12GraphicsCommandList6 *commandList;
        hr = engine->d3d12_device->CreateCommandList(engine->d3d12_device->GetNodeCount(),D3D12_COMMAND_LIST_TYPE_DIRECT,bufferAllocator.Get(),NULL,IID_PPV_ARGS(&commandList));
        if(FAILED(hr)){
             MessageBoxA(GetForegroundWindow(),"Failed to Create Command List","NOTE",MB_OK);
            exit(1);
        };
        return std::make_shared<GED3D12CommandBuffer>(commandList,this);

    };

    void GED3D12CommandQueue::reset(){
        HRESULT hr;
        hr = bufferAllocator->Reset();
        if(FAILED(hr)){
            exit(1);
        };
    };
_NAMESPACE_END_