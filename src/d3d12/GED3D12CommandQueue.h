#include "omegaGTE/GECommandQueue.h"
#include "GED3D12.h"

#ifndef OMEGAGTE_GED3D12COMMANDQUEUE_H
#define OMEGAGTE_GED3D12COMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GED3D12CommandQueue;

    class GED3D12CommandBuffer : public GECommandBuffer {
        ComPtr<ID3D12GraphicsCommandList6> commandList;
        GED3D12CommandQueue *parentQueue;
        bool inComputePass;
        bool inBlitPass;
        std::vector<ID3D12DescriptorHeap *> descriptorHeapBuffer;
    public:
        void startBlitPass();
        void finishBlitPass();

        void startRenderPass(const GERenderPassDescriptor &desc);
        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState);
        void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned int index);
        void setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned int index);
        void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned int index);
        void setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned int index);
        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx);
        void setViewports(std::vector<GEViewport> viewports);
        void setScissorRects(std::vector<GEScissorRect> scissorRects);
        void finishRenderPass();

        void startComputePass(const GEComputePassDescriptor &desc);
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState);
        void finishComputePass();

        void commitToQueue();
        GED3D12CommandBuffer(ID3D12GraphicsCommandList6 *commandList,GED3D12CommandQueue *parentQueue);
        void reset();
    };

    class GED3D12CommandQueue : public GECommandQueue {
        GED3D12Engine *engine;
        std::vector<ID3D12GraphicsCommandList6 *> commandLists;
        ComPtr<ID3D12CommandQueue> commandQueue;
        ComPtr<ID3D12CommandAllocator> bufferAllocator;
        unsigned currentCount;
        friend class GED3D12Engine;
        friend class GED3D12CommandBuffer;
    public:
        void commitToGPU();
        void reset();
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GED3D12CommandQueue(GED3D12Engine *engine,unsigned size);
    };
_NAMESPACE_END_
#endif
