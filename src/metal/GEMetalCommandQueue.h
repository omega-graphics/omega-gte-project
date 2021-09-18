#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include "GEMetal.h"
#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGTE_METAL_GEMETALCOMMANDQUEUE_H
#define OMEGAGTE_METAL_GEMETALCOMMANDQUEUE_H

_NAMESPACE_BEGIN_

    class GEMetalCommandQueue;
    class GEMetalRenderPipelineState;
    class GEMetalComputePipelineState;

    class GEMetalCommandBuffer final : public GECommandBuffer {
        id<MTLRenderCommandEncoder> rp = nil;
        id<MTLComputeCommandEncoder> cp = nil;
        id<MTLBlitCommandEncoder> bp = nil;
        GEMetalCommandQueue *parentQueue = nullptr;

        GEMetalRenderPipelineState *renderPipelineState = nullptr;
        GEMetalRenderPipelineState *computePipelineState = nullptr;

        friend class GEMetalCommandQueue;
        void _present_drawable(NSSmartPtr & drawable);
        void _commit();
    public:
        NSSmartPtr buffer;
        void startBlitPass() override;
        void finishBlitPass() override;
        
        void startRenderPass(const GERenderPassDescriptor &desc) override;
        void setVertexBuffer(SharedHandle<GEBuffer> &buffer) override;
        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState) override;
        void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned index) override;
        void setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned index) override;
        void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned index) override;
        void setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned index) override;
        void setViewports(std::vector<GEViewport> viewports) override;
        void setScissorRects(std::vector<GEScissorRect> scissorRects) override;
        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned vertexCount, size_t startIdx) override;
        void finishRenderPass() override;
        
        void startComputePass(const GEComputePassDescriptor &desc) override;
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState) override;
//        void setResourceConstAtComputeFunc(SharedHandle<GEBuffer> &buffer, unsigned index) override;
//        void setResourceConstAtComputeFunc(SharedHandle<GETexture> &texture, unsigned index) override;
        void finishComputePass() override;

        GEMetalCommandBuffer(GEMetalCommandQueue *parentQueue);
        ~GEMetalCommandBuffer();
        void reset() override;
    };

    class GEMetalCommandQueue : public GECommandQueue {
        NSSmartPtr commandQueue;

        std::vector<SharedHandle<GECommandBuffer>> commandBuffers;

        
        friend class GEMetalCommandBuffer;
        friend class GEMetalNativeRenderTarget;
        friend class GEMetalTextureRenderTarget;
        void commitToGPUAndPresent(NSSmartPtr & drawable);
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEMetalCommandQueue(NSSmartPtr & commandQueue,unsigned size);
        ~GEMetalCommandQueue();
        void submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer);
        void commitToGPU();
    };
_NAMESPACE_END_

#endif
