#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGTE_METAL_GEMETALCOMMANDQUEUE_H
#define OMEGAGTE_METAL_GEMETALCOMMANDQUEUE_H

_NAMESPACE_BEGIN_

    class GEMetalCommandQueue;
    class GEMetalCommandBuffer : public GECommandBuffer {
        id<MTLCommandBuffer> buffer;
        id<MTLRenderCommandEncoder> rp = nil;
        id<MTLComputeCommandEncoder> cp = nil;
        GEMetalCommandQueue *parentQueue;
    public:
        void startRenderPass(const GERenderPassDescriptor &desc);
        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState);
        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned vertexCount, size_t startIdx);
        void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned index);
        void setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned index);
        void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned index);
        void setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned index);
        void finishRenderPass();
        
        void startComputePass(const GEComputePassDescriptor &desc);
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState);
        void finishComputePass();
        GEMetalCommandBuffer(id<MTLCommandBuffer> buffer,GEMetalCommandQueue *parentQueue);
        void commitToQueue();
        void reset();
    };

    class GEMetalCommandQueue : public GECommandQueue {
        id<MTLCommandQueue> commandQueue;
        NSMutableArray<id<MTLCommandBuffer>> * commandBuffers;
        friend class GEMetalCommandBuffer;
        friend class GEMetalNativeRenderTarget;
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size);
        ~GEMetalCommandQueue();
        void commitToGPU();
    };
_NAMESPACE_END_

#endif
