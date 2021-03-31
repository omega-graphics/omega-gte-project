#include "GTEBase.h"
#include "GEPipeline.h"
#include "GERenderTarget.h"

#ifndef OMEGAGTE_GECOMMANDQUEUE_H
#define OMEGAGTE_GECOMMANDQUEUE_H

_NAMESPACE_BEGIN_
    struct GERenderPassDescriptor {
        GENativeRenderTarget *nRenderTarget = nullptr;
        GETextureRenderTarget *tRenderTarget = nullptr;
        struct ColorAttachment {
            typedef enum {
                Load,
                LoadPreserve,
                Clear,
                Discard
            } LoadAction;
            LoadAction loadAction;
            struct ClearColor { 
                float r,g,b,a;
                ClearColor(float r,float g,float b,float a);
            };
            ClearColor clearColor;
        };
        ColorAttachment colorAttachment;
    };


    struct GEComputePassDescriptor {

    };


    class GECommandBuffer {
    public:
         /**
         Render Pass
        */
        virtual void startRenderPass(const GERenderPassDescriptor & desc) = 0;
        virtual void setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState) = 0;
        typedef enum : uint8_t {
            Triangle,
            TriangleStrip
        } RenderPassDrawPolygonType;
        virtual void drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t startIdx) = 0;
        virtual void finishRenderPass() = 0;
        /**
         Compute Pass
        */
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
        virtual void present() = 0;
    };
_NAMESPACE_END_

#endif
