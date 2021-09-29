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

    unsigned int GED3D12CommandBuffer::getRootParameterIndexOfResource(unsigned int id, omegasl_shader &shader){
        bool isSRV = false,isUAV = false,isDescriptorTable = false;
        OmegaCommon::ArrayRef<omegasl_shader_layout_desc> layoutArr {shader.pLayout,shader.pLayout + shader.nLayout};
        unsigned relative_index = 0;
        for(auto & l : layoutArr){
            if(l.location == id) {
                relative_index = l.gpu_relative_loc;
                if (l.type == OMEGASL_SHADER_BUFFER_DESC || l.type == OMEGASL_SHADER_TEXTURE2D_DESC || l.type == OMEGASL_SHADER_TEXTURE3D_DESC) {
                    if(l.io_mode == OMEGASL_SHADER_DESC_IO_IN){
                        isSRV = true;
                    }
                    else {
                        isUAV = true;
                    }
                }
                else {
                    isDescriptorTable = true;
                }
                break;
            }
        }

        unsigned idx = 0;
        for(;idx < currentRootSignature->NumParameters;idx++){
            auto & param = currentRootSignature->pParameters[idx];
            if(param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV && isSRV){
                if(param.Descriptor.ShaderRegister == relative_index){
                    break;
                }
            }
            else if(param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV && isUAV){
                if(param.Descriptor.ShaderRegister == relative_index){
                    break;
                }
            }
            else if(param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && isDescriptorTable){
                if(param.DescriptorTable.pDescriptorRanges->BaseShaderRegister == relative_index){
                    break;
                }
            }
        }
        return idx;
    }

    void GED3D12CommandBuffer::startBlitPass(){
        inBlitPass = true;
    };

    void GED3D12CommandBuffer::copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest) {
        assert(inBlitPass && "Not in Blit Pass! Exiting...");
        auto *srcText = (GED3D12Texture *)src.get(),*destText = (GED3D12Texture *)dest.get();
        commandList->CopyResource(destText->resource.Get(),srcText->resource.Get());
    }

    void GED3D12CommandBuffer::copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest,const TextureRegion & region,const GPoint3D & destCoord) {
        assert(inBlitPass && "Not in Blit Pass! Exiting...");
        auto *srcText = (GED3D12Texture *)src.get(),*destText = (GED3D12Texture *)dest.get();
        CD3DX12_TEXTURE_COPY_LOCATION srcLoc(srcText->resource.Get()),
                                        destLoc(destText->resource.Get());
        LONG top_pos = LONG(region.h) - LONG(region.y);
        CD3DX12_BOX _region ((LONG)region.x,top_pos,LONG(region.x + region.w),LONG(top_pos + region.h));
        commandList->CopyTextureRegion(&destLoc,(UINT)destCoord.x,(UINT)destCoord.y,(UINT)destCoord.z,&srcLoc,&_region);
    }

    void GED3D12CommandBuffer::finishBlitPass(){
        inBlitPass = false;
    };

    void GED3D12CommandBuffer::startRenderPass(const GERenderPassDescriptor &desc){
        inRenderPass = true;
        assert(!inComputePass && "Cannot start a Render Pass while in a compute pass.");
        D3D12_RENDER_PASS_RENDER_TARGET_DESC rt_desc;
        CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle;

        if(desc.nRenderTarget) {
            auto *nativeRenderTarget = (GED3D12NativeRenderTarget *)desc.nRenderTarget;
            if(desc.multisampleResolve){
                auto resolveTexture = (GED3D12Texture *)desc.resolveDesc.multiSampleTextureSrc.get();
                cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(resolveTexture->rtvDescHeap->GetCPUDescriptorHandleForHeapStart());
                D3D12_RESOURCE_STATES resource_state;
                if(firstRenderPass) {
                    resource_state = D3D12_RESOURCE_STATE_PRESENT;
                }
                else {
                    resource_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
                }
                auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                        nativeRenderTarget->renderTargets[nativeRenderTarget->frameIndex], resource_state,
                        D3D12_RESOURCE_STATE_RESOLVE_DEST);
                commandList->ResourceBarrier(1, &barrier);
            }
            else {
                cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
                        nativeRenderTarget->descriptorHeapForRenderTarget->GetCPUDescriptorHandleForHeapStart());
                if(firstRenderPass) {
                    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                            nativeRenderTarget->renderTargets[nativeRenderTarget->frameIndex],
                            D3D12_RESOURCE_STATE_PRESENT,
                            D3D12_RESOURCE_STATE_RENDER_TARGET);

                    commandList->ResourceBarrier(1, &barrier);
                }
            }
            currentTarget.native = nativeRenderTarget;
        }
        else if(desc.tRenderTarget){
            auto *textureRenderTarget = (GED3D12TextureRenderTarget *)desc.tRenderTarget;
            cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(textureRenderTarget->descriptorHeapForRenderTarget->GetCPUDescriptorHandleForHeapStart());
            currentTarget.texture = textureRenderTarget;
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

        hasMultisampleDesc = desc.multisampleResolve;
        if(desc.multisampleResolve){
            typedef decltype(desc.resolveDesc) MSResolveDesc;
            multisampleResolveDesc = new MSResolveDesc;
            memcpy_s(multisampleResolveDesc,sizeof(MSResolveDesc),&desc.resolveDesc,sizeof(MSResolveDesc));
        }
        
        commandList->BeginRenderPass(1,&rt_desc,nullptr,D3D12_RENDER_PASS_FLAG_NONE);
        if(firstRenderPass){
            firstRenderPass = false;
        }
    };

    void GED3D12CommandBuffer::setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState){
         assert(!inComputePass && "Cannot set Render Pipeline State while in Compute Pass");
        auto *d3d12_pipeline_state = (GED3D12RenderPipelineState *)pipelineState.get();
        commandList->SetPipelineState(d3d12_pipeline_state->pipelineState.Get());
        currentRenderPipeline = d3d12_pipeline_state;
        commandList->SetGraphicsRootSignature(d3d12_pipeline_state->rootSignature.Get());
        currentRootSignature = &d3d12_pipeline_state->rootSignatureDesc;
    };

    void GED3D12CommandBuffer::bindResourceAtVertexShader(SharedHandle<GEBuffer> &buffer, unsigned int index){
        assert((!inComputePass && !inBlitPass) && "Cannot set Resource Const at a Vertex Func when not in render pass");
        GED3D12Buffer *d3d12_buffer = (GED3D12Buffer *)buffer.get();
        commandList->SetGraphicsRootShaderResourceView(getRootParameterIndexOfResource(index,currentRenderPipeline->vertexShader->internal),d3d12_buffer->buffer->GetGPUVirtualAddress());
        descriptorHeapBuffer.push_back(d3d12_buffer->bufferDescHeap.Get());
    };

    void GED3D12CommandBuffer::bindResourceAtVertexShader(SharedHandle<GETexture> &texture, unsigned int index){
         assert((!inComputePass && !inBlitPass) &&"Cannot set Resource Const at a Vertex Func when not in render pass");
        GED3D12Texture *d3d12_texture = (GED3D12Texture *)texture.get();
        commandList->SetGraphicsRootDescriptorTable(getRootParameterIndexOfResource(index,currentRenderPipeline->vertexShader->internal),d3d12_texture->srvDescHeap->GetGPUDescriptorHandleForHeapStart());
        descriptorHeapBuffer.push_back(d3d12_texture->srvDescHeap.Get());
    };

    void GED3D12CommandBuffer::bindResourceAtFragmentShader(SharedHandle<GEBuffer> &buffer, unsigned int index){
         assert((!inComputePass && !inBlitPass) && "Cannot set Resource Const a Fragment Func when not in render pass");
        GED3D12Buffer *d3d12_buffer = (GED3D12Buffer *)buffer.get();
        commandList->SetGraphicsRootShaderResourceView(getRootParameterIndexOfResource(index,currentRenderPipeline->fragmentShader->internal),d3d12_buffer->buffer->GetGPUVirtualAddress());
        descriptorHeapBuffer.push_back(d3d12_buffer->bufferDescHeap.Get());
    };

    void GED3D12CommandBuffer::bindResourceAtFragmentShader(SharedHandle<GETexture> &texture, unsigned int index){
         assert((!inComputePass && !inBlitPass) && "Cannot set Resource Const a Fragment Func when not in render pass");
         auto *d3d12_texture = (GED3D12Texture *)texture.get();
         commandList->SetGraphicsRootDescriptorTable(getRootParameterIndexOfResource(index,currentRenderPipeline->fragmentShader->internal),d3d12_texture->srvDescHeap->GetGPUDescriptorHandleForHeapStart());
         descriptorHeapBuffer.push_back(d3d12_texture->srvDescHeap.Get());
    };

    void GED3D12CommandBuffer::setViewports(std::vector<GEViewport> viewports){
        std::vector<D3D12_VIEWPORT> d3d12_viewports;
        auto viewports_it = viewports.begin();
        while(viewports_it != viewports.end()){
            GEViewport & viewport = *viewports_it;
            GRect rect {};
            if(currentTarget.native != nullptr) {
                RECT rc;
                GetClientRect(currentTarget.native->hwnd, &rc);
                rect.pos.x = (float)rc.left;
                rect.pos.y = (float)rc.top;
                rect.w = float(rc.right - rc.left);
                rect.h = float(rc.bottom - rc.top);
            }
            else {
                rect.pos.x = 0;
                rect.pos.y = 0;
                auto res_desc = currentTarget.texture->renderTargetView->GetDesc();
                rect.w = (float)res_desc.Width;
                rect.h = (float)res_desc.Height;
            }

            CD3DX12_VIEWPORT v(viewport.x,rect.h - (viewport.y + viewport.height),viewport.width,viewport.height,viewport.nearDepth,viewport.farDepth);
            d3d12_viewports.push_back(v);
            ++viewports_it;
        };
        commandList->RSSetViewports(d3d12_viewports.size(),d3d12_viewports.data());
    };

    void GED3D12CommandBuffer::setScissorRects(std::vector<GEScissorRect> scissorRects){
        std::vector<D3D12_RECT> d3d12_rects;
        auto rects_it = scissorRects.begin();
        while(rects_it != scissorRects.end()){
            GEScissorRect & _rect = *rects_it;

            GRect rect {};
            if(currentTarget.native != nullptr) {
                RECT rc;
                GetClientRect(currentTarget.native->hwnd, &rc);
                rect.pos.x = (float)rc.left;
                rect.pos.y = (float)rc.top;
                rect.w = float(rc.right - rc.left);
                rect.h = float(rc.bottom - rc.top);
            }
            else {
                rect.pos.x = 0;
                rect.pos.y = 0;
                auto res_desc = currentTarget.texture->renderTargetView->GetDesc();
                rect.w = (float)res_desc.Width;
                rect.h = (float)res_desc.Height;
            }

            float top_coord = rect.h - (_rect.height + _rect.y);

            CD3DX12_RECT r((LONG)_rect.x,(LONG)top_coord,LONG(_rect.width + _rect.x),LONG(top_coord + _rect.height));
            d3d12_rects.push_back(r);
            ++rects_it;
        };
        commandList->RSSetScissorRects(d3d12_rects.size(),d3d12_rects.data());
    };

    void GED3D12CommandBuffer::setVertexBuffer(SharedHandle<GEBuffer> &buffer)
    {
        auto *b = (GED3D12Buffer *)buffer.get();
        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = b->buffer->GetGPUVirtualAddress();
        view.SizeInBytes = UINT(b->size());
        view.StrideInBytes = 1;
        commandList->IASetVertexBuffers(0,1,&view);
    };

    void GED3D12CommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx){
        assert(!inComputePass && "Cannot Draw Polygons while in Compute Pass");
        D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::Triangle){
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
        else if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::TriangleStrip){
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        };
        commandList->IASetPrimitiveTopology(topology);
        commandList->DrawInstanced(vertexCount,1,startIdx,0);
    };

    void GED3D12CommandBuffer::finishRenderPass(){
        assert(inRenderPass && "");
        commandList->SetDescriptorHeaps(descriptorHeapBuffer.size(),descriptorHeapBuffer.data());
        commandList->EndRenderPass();
        if(hasMultisampleDesc) {
            ID3D12Resource *destTarget;
            if(currentTarget.native != nullptr){
                destTarget = currentTarget.native->renderTargets[currentTarget.native->frameIndex];
            }
            else {
                destTarget = currentTarget.texture->renderTargetView.Get();
            }
            auto destTargetDesc = destTarget->GetDesc();
            auto resolveTexture = ((GED3D12Texture *)multisampleResolveDesc->multiSampleTextureSrc.get())->resource.Get();
            auto resolveTextureDesc = resolveTexture->GetDesc();

            D3D12_RESOURCE_BARRIER
            barrierA = CD3DX12_RESOURCE_BARRIER::Transition(resolveTexture,D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
            barrierB = CD3DX12_RESOURCE_BARRIER::Transition(destTarget,D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_RESOLVE_DEST);
            D3D12_RESOURCE_BARRIER barriers[] = {barrierA,barrierB};
            commandList->ResourceBarrier(2,barriers);
            commandList->ResolveSubresource(destTarget,
                                            D3D12CalcSubresource(0,0,0,destTargetDesc.MipLevels,destTargetDesc.DepthOrArraySize),
                                            resolveTexture,
                                            D3D12CalcSubresource(multisampleResolveDesc->slice,0,multisampleResolveDesc->level,resolveTextureDesc.MipLevels,resolveTextureDesc.DepthOrArraySize),DXGI_FORMAT_R8G8B8A8_UNORM);
            barrierA = CD3DX12_RESOURCE_BARRIER::Transition(destTarget,D3D12_RESOURCE_STATE_RESOLVE_DEST,D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList->ResourceBarrier(1,&barrierA);
            hasMultisampleDesc = false;
            delete multisampleResolveDesc;
        }
        commandList->ClearState(nullptr);
        descriptorHeapBuffer.clear();
        currentTarget.texture = nullptr;
        currentTarget.native = nullptr;
        currentRenderPipeline = nullptr;
        currentRootSignature = nullptr;
    };

    void GED3D12CommandBuffer::startComputePass(const GEComputePassDescriptor &desc){
        inComputePass = true;
    };

    void GED3D12CommandBuffer::setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState){
        auto *d3d12_pipeline_state = (GED3D12ComputePipelineState *)pipelineState.get();
        commandList->SetPipelineState(d3d12_pipeline_state->pipelineState.Get());
        commandList->SetComputeRootSignature(d3d12_pipeline_state->rootSignature.Get());
        currentComputePipeline = d3d12_pipeline_state;
        currentRootSignature = &d3d12_pipeline_state->rootSignatureDesc;
    };

    void GED3D12CommandBuffer::bindResourceAtComputeShader(SharedHandle<GEBuffer> &buffer, unsigned int id) {
        assert(inComputePass && "");
        auto *d3d12_buffer = (GED3D12Buffer *)buffer.get();
        D3D12_HEAP_PROPERTIES heap_props;
        D3D12_HEAP_FLAGS heapFlags;
        d3d12_buffer->buffer->GetHeapProperties(&heap_props,&heapFlags);
        if(heap_props.Type == D3D12_HEAP_TYPE_UPLOAD){
            commandList->SetGraphicsRootShaderResourceView(getRootParameterIndexOfResource(id,currentComputePipeline->computeShader->internal),d3d12_buffer->buffer->GetGPUVirtualAddress());
        }
        else if(heap_props.Type == D3D12_HEAP_TYPE_READBACK){
            auto resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12_buffer->buffer.Get(),D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            commandList->ResourceBarrier(1,&resource_barrier);
            commandList->SetGraphicsRootUnorderedAccessView(getRootParameterIndexOfResource(id,currentComputePipeline->computeShader->internal),d3d12_buffer->buffer->GetGPUVirtualAddress());
            uavResourceBuffer.push_back(d3d12_buffer->buffer.Get());
        }
        descriptorHeapBuffer.push_back(d3d12_buffer->bufferDescHeap.Get());

    }

    void GED3D12CommandBuffer::bindResourceAtComputeShader(SharedHandle<GETexture> &texture, unsigned int id) {
        assert(inComputePass && "");
        auto *d3d12_texture = (GED3D12Texture *)texture.get();
        D3D12_HEAP_PROPERTIES heap_props;
        D3D12_HEAP_FLAGS heapFlags;
        d3d12_texture->resource->GetHeapProperties(&heap_props,&heapFlags);
        if(heap_props.Type == D3D12_HEAP_TYPE_READBACK){
            auto resource_barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12_texture->resource.Get(),D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            commandList->ResourceBarrier(1,&resource_barrier);
            uavResourceBuffer.push_back(d3d12_texture->resource.Get());
        }
        commandList->SetGraphicsRootDescriptorTable(getRootParameterIndexOfResource(id,currentComputePipeline->computeShader->internal),d3d12_texture->srvDescHeap->GetGPUDescriptorHandleForHeapStart());
        descriptorHeapBuffer.push_back(d3d12_texture->srvDescHeap.Get());
    }

    void GED3D12CommandBuffer::dispatchThreads(unsigned int x, unsigned int y, unsigned int z) {
        assert(inComputePass && "");
        commandList->SetDescriptorHeaps(descriptorHeapBuffer.size(),descriptorHeapBuffer.data());
        commandList->Dispatch(x,y,z);
        descriptorHeapBuffer.clear();
    }

    void GED3D12CommandBuffer::finishComputePass(){
        OmegaCommon::Vector<D3D12_RESOURCE_BARRIER> uavBarriers;
        for(auto & r : uavResourceBuffer){
            auto res_desc = r->GetDesc();
            if(res_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D || res_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D || res_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D){
                uavBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(r,D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
            }
            else {
                uavBarriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(r,D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
            }
        }
        if(!uavBarriers.empty()){
            commandList->ResourceBarrier(uavBarriers.size(),uavBarriers.data());
        }
        commandList->ClearState(nullptr);
        uavResourceBuffer.clear();
        inComputePass = false;
        currentComputePipeline = nullptr;
        currentRootSignature = nullptr;
    };

    void GED3D12CommandBuffer::waitForFence(SharedHandle<GEFence> &fence,unsigned val) {
        auto _fence = (GED3D12Fence *)fence.get();
        parentQueue->commandQueue->Wait(_fence->fence.Get(),val);
    }

    void GED3D12CommandBuffer::signalFence(SharedHandle<GEFence> &fence,unsigned val) {
        auto _fence = (GED3D12Fence *)fence.get();
        parentQueue->commandQueue->Signal(_fence->fence.Get(),val);
    }

    GED3D12CommandBuffer::~GED3D12CommandBuffer(){
        
    };

    // void GED3D12CommandBuffer::commitToQueue(){
    //     HRESULT hr;
    //     hr = commandList->Close();
        
    //     parentQueue->commandLists.push_back(commandList.Get());
    // };

    void GED3D12CommandQueue::submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer){
        HRESULT hr;
        auto d3d12_buffer = (GED3D12CommandBuffer *)commandBuffer.get();
        d3d12_buffer->closed = true;
        
        commandLists.push_back(d3d12_buffer->commandList.Get());
    };

    void GED3D12CommandBuffer::reset(){
        closed = false;
        firstRenderPass = true;
        commandList->Reset(parentQueue->bufferAllocator.Get(),nullptr);
    };

    void GED3D12CommandQueue::commitToGPU(){
        for(auto & cl : commandLists){
            cl->Close();
        }
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

    ID3D12GraphicsCommandList6 *GED3D12CommandQueue::getLastCommandList() {
        return commandLists.back();
    }

    GED3D12CommandQueue::~GED3D12CommandQueue(){

    };
_NAMESPACE_END_