#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGTE_METAL_GEMETALCOMMANDQUEUE_H
#define OMEGAGTE_METAL_GEMETALCOMMANDQUEUE_H

_NAMESPACE_BEGIN_

    class GEMetalCommandQueue;
    class GEMetalCommandBuffer final : public GECommandBuffer {
        id<MTLRenderCommandEncoder> rp = nil;
        id<MTLComputeCommandEncoder> cp = nil;
        id<MTLBlitCommandEncoder> bp = nil;
        GEMetalCommandQueue *parentQueue = nullptr;
        friend class GEMetalCommandQueue;
        void __present_drawable(id<CAMetalDrawable> drawable);
        void __commit();
    public:
        id<MTLCommandBuffer> buffer = nil;
        void startBlitPass();
        void finishBlitPass();
        
        void startRenderPass(const GERenderPassDescriptor &desc);
        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState);
        void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned index);
        void setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned index);
        void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned index);
        void setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned index);
        void setViewports(std::vector<GEViewport> viewports);
        void setScissorRects(std::vector<GEScissorRect> scissorRects);
        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned vertexCount, size_t startIdx);
        void finishRenderPass();
        
        void startComputePass(const GEComputePassDescriptor &desc);
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState);
        void setResourceConstAtComputeFunc(SharedHandle<GEBuffer> &buffer, unsigned index);
        void setResourceConstAtComputeFunc(SharedHandle<GETexture> &texture, unsigned index);
        void finishComputePass();
        GEMetalCommandBuffer(id<MTLCommandBuffer> buffer,GEMetalCommandQueue *parentQueue);
        ~GEMetalCommandBuffer();
        void reset();
    };

    class GEMetalCommandQueue : public GECommandQueue {
        id<MTLCommandQueue> commandQueue = nil;

        std::vector<GEMetalCommandBuffer *> commandBuffers;

        
        friend class GEMetalCommandBuffer;
        friend class GEMetalNativeRenderTarget;
        friend class GEMetalTextureRenderTarget;
        void commitToGPUAndPresent(id<CAMetalDrawable> drawable);
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size);
        ~GEMetalCommandQueue();
        void submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer);
        void commitToGPU();
    };
_NAMESPACE_END_

#endif
