#include "GTEBase.h"
#include "GEPipeline.h"
                                                   

// #ifdef TARGET_DIRECTX
// #include <windows.h>
// #endif


#ifndef OMEGAGTE_GERENDERTARGET_H
#define OMEGAGTE_GERENDERTARGET_H

_NAMESPACE_BEGIN_
    class GECommandBuffer;
    class GEBuffer;
    class GETexture;
    struct GEViewport;
    struct GEScissorRect;

    class  OMEGAGTE_EXPORT GERenderTarget {
    public:
        struct OMEGAGTE_EXPORT RenderPassDesc {
            struct OMEGAGTE_EXPORT ColorAttachment {
                typedef enum {
                    Load,
                    LoadPreserve,
                    Clear,
                    Discard
                } LoadAction;
                LoadAction loadAction;
                struct OMEGAGTE_EXPORT ClearColor {
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
          
        public:
            typedef enum : uint8_t {
                Native,
                Texture
            } GERTType;
            GERTType renderTargetTy;
            /// Do NOT CALL THIS CONSTRUCTOR!!!
            CommandBuffer(GERenderTarget *renderTarget,GERTType type,SharedHandle<GECommandBuffer> commandBuffer);
            friend SharedHandle<CommandBuffer> commandBuffer();
            void startRenderPass(const RenderPassDesc & desc);
            void setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState);

            void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> & buffer,unsigned index);
            void setResourceConstAtVertexFunc(SharedHandle<GETexture> & texture,unsigned index);
        
            void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> & buffer,unsigned index);
            void setResourceConstAtFragmentFunc(SharedHandle<GETexture> & texture,unsigned index);
        
            void setViewports(std::vector<GEViewport> viewport);
            void setScissorRects(std::vector<GEScissorRect> scissorRects);
        

            typedef enum : uint8_t {
                Triangle,
                TriangleStrip
            } RenderPassDrawPolygonType;
            void drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t start);
            void endRenderPass();
            
            void startComputePass(SharedHandle<GEComputePipelineState> & computePipelineState);
            
            void endComputePass();
            
            void reset();
        };
        virtual SharedHandle<CommandBuffer> commandBuffer() = 0;
        virtual void submitCommandBuffer(SharedHandle<CommandBuffer> & commandBuffer) = 0;
    };
    class  OMEGAGTE_EXPORT GENativeRenderTarget : public GERenderTarget {
        public:
        virtual void commitAndPresent() = 0;
        #ifdef _WIN32 
        /// @returns IDXGISwapChain1 * if D3D11, else IDXGISwapChain3 *
        virtual void *getSwapChain() = 0;
        #endif
     };
     class  OMEGAGTE_EXPORT GETextureRenderTarget : public GERenderTarget {
         public:
         virtual void commit() = 0;
     };

_NAMESPACE_END_

#endif
