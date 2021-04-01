#include "omegaGTE/GECommandQueue.h"
#include "GED3D12.h"

#ifndef OMEGAGRAPHICSENGINE_GED3D12COMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_GED3D12COMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GED3D12CommandQueue;

    class GED3D12CommandBuffer : public GECommandBuffer {
        ComPtr<ID3D12GraphicsCommandList6> commandList;
        GED3D12CommandQueue *parentQueue;
    public:
        void startRenderPass(const GERenderPassDescriptor &desc);
        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState);
        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx);
        void finishRenderPass();

        void startComputePass(const GEComputePassDescriptor &desc);
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState);
        void finishComputePass();

        void commitToQueue();
        GED3D12CommandBuffer(ID3D12GraphicsCommandList6 *commandList,GED3D12CommandQueue *parentQueue);
    };

    class GED3D12CommandQueue : public GECommandQueue {
        GED3D12Engine *engine;
        ComPtr<ID3D12CommandQueue> commandQueue;
        unsigned currentCount;
        friend class GED3D12Engine;
        friend class GED3D12CommandBuffer;
    public:
        void present();
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GED3D12CommandQueue(GED3D12Engine *engine,unsigned size);
    };
_NAMESPACE_END_
#endif
