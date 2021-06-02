#include "GTEBase.h"
#include "GEPipeline.h"
                                                   

// #ifdef TARGET_DIRECTX
// #include <windows.h>
// #endif


#ifndef OMEGAGTE_GERENDERTARGET_H
#define OMEGAGTE_GERENDERTARGET_H

_NAMESPACE_BEGIN_
    class GECommandBuffer;

    class  OMEGAGTE_EXPORT GERenderTarget {
    public:
        struct RenderPassDesc {
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
            ColorAttachment * colorAttachment;
        };
        class OMEGAGTE_EXPORT CommandBuffer {
            GERenderTarget *renderTargetPtr;
            SharedHandle<GECommandBuffer> commandBuffer;
            #ifdef TARGET_DIRECTX
            friend class GED3D12NativeRenderTarget;
            friend class GED3D12TextureRenderTarget;
            #endif
            
            #ifdef TARGET_METAL
            friend class GEMetalNativeRenderTarget;
            friend class GEMetalTextureRenderTarget;
            #endif
        
            /// Do NOT CALL THESE CONSTRUCTORS!!!
            typedef enum : uint8_t {
                Native,
                Texture
            } GERTType;
            GERTType renderTargetTy;
            CommandBuffer(GERenderTarget *renderTarget,GERTType type,const SharedHandle<GECommandBuffer> & commandBuffer);
        public:
            friend SharedHandle<CommandBuffer> commandBuffer();
            void startRenderPass(const RenderPassDesc & desc);
            void setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState);
            typedef enum : uint8_t {
                Triangle,
                TriangleStrip
            } RenderPassDrawPolygonType;
            void drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t start);
            void endRenderPass();
            
            void startComputePass(SharedHandle<GEComputePipelineState> & computePipelineState);
            
            void endComputePass();
            
            void schedule();
            
            void reset();
        };
        virtual SharedHandle<CommandBuffer> commandBuffer() = 0;
    };
    class  OMEGAGTE_EXPORT GENativeRenderTarget : public GERenderTarget {
        public:
        virtual void commitAndPresent() = 0;
     };
     class  OMEGAGTE_EXPORT GETextureRenderTarget : public GERenderTarget {
         public:
         virtual void commit() = 0;
     };

_NAMESPACE_END_

#endif
