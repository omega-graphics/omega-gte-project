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
        friend class GED3D12CommandQueue;

        struct {
            GED3D12NativeRenderTarget *native = nullptr;
            GED3D12TextureRenderTarget *texture = nullptr;
        } currentTarget;

       
    public:

        void startBlitPass() override;
        void copyTextureToTexture(SharedHandle<GETexture> & src,SharedHandle<GETexture> & dest);
        void copyTextureToTexture(SharedHandle<GETexture> & src,SharedHandle<GETexture> & dest,const TextureRegion & region,const GPoint3D & destCoord);
        void finishBlitPass() override;

        void startRenderPass(const GERenderPassDescriptor &desc) override;
        void setVertexBuffer(SharedHandle<GEBuffer> &buffer) override;
        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState) override;
        void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned int index) override;
        void setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned int index) override;
        void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned int index) override;
        void setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned int index) override;
       
        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx) override;
        void setViewports(std::vector<GEViewport> viewports) override;
        void setScissorRects(std::vector<GEScissorRect> scissorRects) override;
        void finishRenderPass() override;

        void startComputePass(const GEComputePassDescriptor &desc) override;
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState) override;
        void finishComputePass() override;

        GED3D12CommandBuffer(ID3D12GraphicsCommandList6 *commandList,GED3D12CommandQueue *parentQueue);
        void reset() override;
        void waitForFence(SharedHandle<GEFence> &fence,unsigned val) override;
        void signalFence(SharedHandle<GEFence> &fence,unsigned val) override;
        ~GED3D12CommandBuffer();
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
        ID3D12GraphicsCommandList6 * getLastCommandList();
        void commitToGPU();
        void reset();
        void submitCommandBuffer(SharedHandle<GECommandBuffer> & commandBuffer);
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GED3D12CommandQueue(GED3D12Engine *engine,unsigned size);
        ~GED3D12CommandQueue();
    };
_NAMESPACE_END_
#endif
