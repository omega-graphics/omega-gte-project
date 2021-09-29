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
        GEMetalComputePipelineState *computePipelineState = nullptr;

        friend class GEMetalCommandQueue;
        unsigned getResourceLocalIndexFromGlobalIndex(unsigned _id,omegasl_shader & shader);
        void _present_drawable(NSSmartPtr & drawable);
        void _commit();
    public:
        NSSmartPtr buffer;
        void startBlitPass() override;
        void copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest) override;
        void copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest, const TextureRegion &region, const GPoint3D &destCoord) override;
        void finishBlitPass() override;
        
        void startRenderPass(const GERenderPassDescriptor &desc) override;
        void setVertexBuffer(SharedHandle<GEBuffer> &buffer) override;
        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState) override;
        void bindResourceAtVertexShader(SharedHandle<GEBuffer> &buffer, unsigned id) override;
        void bindResourceAtVertexShader(SharedHandle<GETexture> &texture, unsigned id) override;
        void bindResourceAtFragmentShader(SharedHandle<GEBuffer> &buffer, unsigned id) override;
        void bindResourceAtFragmentShader(SharedHandle<GETexture> &texture, unsigned id) override;
        void setViewports(std::vector<GEViewport> viewports) override;
        void setScissorRects(std::vector<GEScissorRect> scissorRects) override;
        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned vertexCount, size_t startIdx) override;
        void finishRenderPass() override;
        
        void startComputePass(const GEComputePassDescriptor &desc) override;
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState) override;
        void bindResourceAtComputeShader(SharedHandle<GEBuffer> &buffer, unsigned id) override;
        void bindResourceAtComputeShader(SharedHandle<GETexture> &texture, unsigned id) override;
        void dispatchThreads(unsigned int x, unsigned int y, unsigned int z) override;
        void finishComputePass() override;

        void waitForFence(SharedHandle<GEFence> &fence, unsigned int val) override;
        void signalFence(SharedHandle<GEFence> &fence, unsigned int val) override;

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
