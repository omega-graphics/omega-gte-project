#include "GED3D12CommandQueue.h"

_NAMESPACE_BEGIN_
    // GED3D12CommandBuffer::GED3D12CommandBuffer(){};
    // void GED3D12CommandBuffer::commitToBuffer(){};
    GED3D12CommandQueue::GED3D12CommandQueue(GED3D12Engine *engine,unsigned size):GECommandQueue(size),engine(engine),currentCount(0){
        HRESULT hr;
        D3D12_COMMAND_QUEUE_DESC desc;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = engine->d3d12_device->GetNodeCount();
        desc.Priority = 0;
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = engine->d3d12_device->CreateCommandQueue(&desc,IID_PPV_ARGS(commandQueue.GetAddressOf()));
        if(FAILED(hr)){
            exit(1);
        };

    };

    GED3D12CommandBuffer::GED3D12CommandBuffer(ID3D12GraphicsCommandList6 *commandList,GED3D12CommandQueue *parentQueue):commandList(commandList),parentQueue(parentQueue){
        
    };

    void GED3D12CommandBuffer::startRenderPass(const GERenderPassDescriptor &desc){
        D3D12_RENDER_PASS_RENDER_TARGET_DESC rt_desc;
        commandList->BeginRenderPass(1,&rt_desc,nullptr,D3D12_RENDER_PASS_FLAG_NONE);
    };

    void GED3D12CommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx){
        D3D12_PRIMITIVE_TOPOLOGY topology;
        if(polygonType == Triangle){
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
        else if(polygonType == TriangleStrip){
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        };
        commandList->IASetPrimitiveTopology(topology);
        commandList->DrawInstanced(1,vertexCount,startIdx,0);
    };

    void GED3D12CommandBuffer::finishRenderPass(){
        commandList->EndRenderPass();
    };

    void GED3D12CommandBuffer::commitToQueue(){
        HRESULT hr;
        hr = commandList->Close();
        parentQueue->commandQueue->ExecuteCommandLists(1,(ID3D12CommandList * const *)commandList.GetAddressOf());
    };

    void GED3D12CommandQueue::present(){

    };

    SharedHandle<GECommandBuffer> GED3D12CommandQueue::getAvailableBuffer(){
        HRESULT hr;
        ID3D12GraphicsCommandList4 *commandList;
        hr = engine->d3d12_device->CreateCommandList(engine->d3d12_device->GetNodeCount(),D3D12_COMMAND_LIST_TYPE_DIRECT,engine->bufferAllocator.Get(),NULL,IID_PPV_ARGS(&commandList));
        if(FAILED(hr)){
            exit(1);
        };
        return std::make_shared<GED3D12CommandBuffer>(commandList,this);

    };
_NAMESPACE_END_