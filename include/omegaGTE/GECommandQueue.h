#include "GTEBase.h"
#include "GEPipeline.h"
#include "GERenderTarget.h"

#ifndef OMEGAGTE_GECOMMANDQUEUE_H
#define OMEGAGTE_GECOMMANDQUEUE_H

_NAMESPACE_BEGIN_
    struct GERenderPassDescriptor {
        GENativeRenderTarget *nRenderTarget = nullptr;
        GETextureRenderTarget *tRenderTarget = nullptr;
        typedef GERenderTarget::RenderPassDesc::ColorAttachment ColorAttachment;
        ColorAttachment *colorAttachment;
    };


    struct GEComputePassDescriptor {

    };


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
