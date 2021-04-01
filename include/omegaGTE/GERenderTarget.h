#include "GTEBase.h"
#include "GEPipeline.h"
                                                   

// #ifdef TARGET_DIRECTX
// #include <windows.h>
// #endif


#ifndef OMEGAGTE_GERENDERTARGET_H
#define OMEGAGTE_GERENDERTARGET_H

_NAMESPACE_BEGIN_
    class GECommandBuffer;

    class GERenderTarget {
    public:
        struct RenderPassDesc {
            SharedHandle<GERenderPipelineState> pipelineState;
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
                ColorAttachment(ClearColor clearColor,LoadAction loadAction);
            };
            ColorAttachment colorAttachment;
        };
        class CommandBuffer {
            GERenderTarget *renderTargetPtr;
            typedef enum : uint8_t {
                Native,
                Texture
            } GERTType;
            GERTType renderTargetTy;
            SharedHandle<GECommandBuffer> commandBuffer;
        public:
            void startRenderPass(const RenderPassDesc & desc);
            typedef enum : uint8_t {
                Triangle,
                TriangleStrip
            } RenderPassDrawPolygonType;
            void drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t start);
            void endRenderPass();
            
            void startComputePass(SharedHandle<GEComputePipelineState> & computePipelineState);
            
            void endComputePass();
            
            void schedule();
        };
        virtual SharedHandle<CommandBuffer> commandBuffer() = 0;
    };
    class GENativeRenderTarget : public GERenderTarget {
        public:
        virtual void commitAndPresent() = 0;
     };
     class GETextureRenderTarget : public GERenderTarget {
         public:
         virtual void commit() = 0;
     };

_NAMESPACE_END_

#endif
