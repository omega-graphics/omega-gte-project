#include "GTEBase.h"
#include "GEPipeline.h"
#include "GERenderTarget.h"
#include "GETexture.h"
#include <vector>

#ifndef OMEGAGTE_GECOMMANDQUEUE_H
#define OMEGAGTE_GECOMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GEBuffer;
    class GEFence;

    struct GEScissorRect;
    struct GEViewport;

    struct  OMEGAGTE_EXPORT GERenderPassDescriptor {
        GENativeRenderTarget *nRenderTarget = nullptr;
        GETextureRenderTarget *tRenderTarget = nullptr;
        typedef GERenderTarget::RenderPassDesc::ColorAttachment ColorAttachment;
        ColorAttachment *colorAttachment;
    };


    struct  OMEGAGTE_EXPORT GEComputePassDescriptor {

    };

    /**
     A Reusable interface for directly uploading commands to a GPU.
     */
    class  OMEGAGTE_EXPORT GECommandBuffer {
        friend class GERenderTarget::CommandBuffer;
    protected:
        typedef GERenderTarget::CommandBuffer::RenderPassDrawPolygonType RenderPassDrawPolygonType;
    private:
        /**
         Blit Pass
         */
        virtual void startBlitPass() = 0;
        virtual void finishBlitPass() = 0;
         /**
         Render Pass
         */
        virtual void startRenderPass(const GERenderPassDescriptor & desc) = 0;
        virtual void setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState) = 0;
        //
        virtual void setVertexBuffer(SharedHandle<GEBuffer> & buffer) = 0;
        
        virtual void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> & buffer,unsigned index) = 0;
        virtual void setResourceConstAtVertexFunc(SharedHandle<GETexture> & texture,unsigned index) = 0;
        
        virtual void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> & buffer,unsigned index) = 0;
        virtual void setResourceConstAtFragmentFunc(SharedHandle<GETexture> & texture,unsigned index) = 0;
        
        virtual void setViewports(std::vector<GEViewport> viewport) = 0;
        virtual void setScissorRects(std::vector<GEScissorRect> scissorRects) = 0;
        
        virtual void drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t startIdx) = 0;
        virtual void finishRenderPass() = 0;
        /**
         Compute Pass
        */
    public:
        virtual void waitForFence(SharedHandle<GEFence> & fence,unsigned val) = 0;
        virtual void signalFence(SharedHandle<GEFence> & fence,unsigned val) = 0;
        virtual void startComputePass(const GEComputePassDescriptor & desc) = 0;
        virtual void finishComputePass() = 0;
        virtual void setComputePipelineState(SharedHandle<GEComputePipelineState> & pipelineState) = 0;
        virtual void reset() = 0;
        virtual ~GECommandBuffer(){};
    };
    class  OMEGAGTE_EXPORT GECommandQueue {
        unsigned size;
    protected:
        unsigned currentlyOccupied = 0;
        GECommandQueue(unsigned size);
    public:
        virtual SharedHandle<GECommandBuffer> getAvailableBuffer() = 0;
        unsigned getSize();
        virtual void submitCommandBuffer(SharedHandle<GECommandBuffer> & commandBuffer) = 0;
        virtual void commitToGPU() = 0;
        virtual ~GECommandQueue(){};
    };
_NAMESPACE_END_

#endif
