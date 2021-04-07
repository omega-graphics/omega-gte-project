#include "GTEBase.h"
#include "GEPipeline.h"
#include "GERenderTarget.h"
#include "GETexture.h"

#ifndef OMEGAGTE_GECOMMANDQUEUE_H
#define OMEGAGTE_GECOMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GEBuffer;

    struct GERenderPassDescriptor {
        GENativeRenderTarget *nRenderTarget = nullptr;
        GETextureRenderTarget *tRenderTarget = nullptr;
        typedef GERenderTarget::RenderPassDesc::ColorAttachment ColorAttachment;
        ColorAttachment *colorAttachment;
    };


    struct GEComputePassDescriptor {

    };

    /**
     A Reusable interface for directly uploading commands to a GPU.
     */
    class GECommandBuffer {
        friend class GERenderTarget::CommandBuffer;
    protected:
        typedef GERenderTarget::CommandBuffer::RenderPassDrawPolygonType RenderPassDrawPolygonType;
    private:
         /**
         Render Pass
        */
        virtual void startRenderPass(const GERenderPassDescriptor & desc) = 0;
        virtual void setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState) = 0;
        
        virtual void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> & buffer,unsigned index) = 0;
        virtual void setResourceConstAtVertexFunc(SharedHandle<GETexture> & texture,unsigned index) = 0;
        
        virtual void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> & buffer,unsigned index) = 0;
        virtual void setResourceConstAtFragmentFunc(SharedHandle<GETexture> & texture,unsigned index) = 0;
        
        virtual void drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t startIdx) = 0;
        virtual void finishRenderPass() = 0;
        /**
         Compute Pass
        */
    public:
        virtual void startComputePass(const GEComputePassDescriptor & desc) = 0;
        virtual void finishComputePass() = 0;
        virtual void setComputePipelineState(SharedHandle<GEComputePipelineState> & pipelineState) = 0;
        virtual void commitToQueue() = 0;
        virtual void reset() = 0;
    };
    class GECommandQueue {
        unsigned size;
    protected:
        unsigned currentlyOccupied = 0;
        GECommandQueue(unsigned size);
    public:
        virtual SharedHandle<GECommandBuffer> getAvailableBuffer() = 0;
        unsigned getSize();
        virtual void commitToGPU() = 0;
    };
_NAMESPACE_END_

#endif
