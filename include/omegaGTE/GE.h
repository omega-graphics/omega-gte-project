

#include <initializer_list>
#if defined(TARGET_DIRECTX)
#include <windows.h>
#include <dxgi1_6.h>
#define DEBUG_ENGINE_PREFIX "GED3D12Engine_Internal"
#endif

#if defined(TARGET_METAL) && defined(__OBJC__)
@class CAMetalLayer;
#define DEBUG_ENGINE_PREFIX "GEMetalEngine_Internal"
#endif

#if defined(TARGET_VULKAN)
#include <X11/Xlib.h>
#define DEBUG_ENGINE_PREFIX "GEVulkanEngine_Internal"
#endif

#define DEBUG_STREAM(message) std::cout << "[" << DEBUG_ENGINE_PREFIX << "] - " << message << std::endl

#include "GTEBase.h"

#ifndef OMEGAGTE_GE_H
#define OMEGAGTE_GE_H

_NAMESPACE_BEGIN_
    struct GTE;
    typedef enum : uint8_t {
        Private,
        Managed,
        Shared
    } StorageOpts;

    typedef struct __GEFunctionInternal GEFunction;
    struct GEFunctionLibrary;
    class GETexture;
    typedef class __GEComputePipelineState GEComputePipelineState;
    typedef class __GERenderPipelineState  GERenderPipelineState;
    class GENativeRenderTarget;
    class GETextureRenderTarget;
    class GECommandQueue;

    struct TextureDescriptor;
    struct TextureRenderTargetDescriptor;
    struct RenderPipelineDescriptor;
    struct ComputePipelineDescriptor;

    struct NativeRenderTargetDescriptor;
    struct TextureRenderTargetDescriptor {
        GRect rect;
    };

    struct OMEGAGTE_EXPORT GEViewport {
        float x,y;
        float width,height;
        float nearDepth,farDepth;
    };

    struct OMEGAGTE_EXPORT GEScissorRect {
        float x,y;
        float width,height;
    };

    struct  OMEGAGTE_EXPORT BufferDescriptor {
        size_t len;
        size_t objectStride;
        StorageOpts opts;
    };

    class  OMEGAGTE_EXPORT GEBuffer {
    public:
        virtual size_t size() = 0;
        virtual void *data() = 0;
        virtual ~GEBuffer(){};
    };

    struct  OMEGAGTE_EXPORT HeapDescriptor {
    public:
        typedef enum : uint8_t {
            Shared,
            Automatic
        } HeapType;
        size_t len;
    };

    class  OMEGAGTE_EXPORT GEHeap {
    public:
        virtual size_t currentSize() = 0;
        /**
         @brief Creates a GEBuffer from a BufferDescriptor.
         @param[in] desc The Buffer Descriptor, which could describe a buffer at any length with any object.
         @returns SharedHandle<GEBuffer>
        */
        virtual SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor & desc) = 0;

        /**
          @brief Creates a GETexture from a TextureDescriptor.
         @param[in] desc The Texture Descriptor,  which could describe a 2D, 3D, 2D-Multisampled,or 3D-Multisampled texture with any given width, height (and depth).
         @returns SharedHandle<GETexture>
        */
        virtual SharedHandle<GETexture> makeTexture(const TextureDescriptor & desc) = 0;
    };

    class  OMEGAGTE_EXPORT GEFence {

    };

    struct OMEGAGTE_EXPORT GEVertex {
        FVector3D pos;
    };

    struct OMEGAGTE_EXPORT GEColoredVertex : public GEVertex {
        FMatrix & color;
    private:
        GEColoredVertex(FVector3D pos,FMatrix & color);
    public:
        static GEColoredVertex FromGPoint3D(GPoint3D & pt,FMatrix & color);
        GEColoredVertex(GEColoredVertex && vertex);
        GEColoredVertex(GEColoredVertex & vertex);
        GEColoredVertex(const GEColoredVertex & vertex);
    };

    struct OMEGAGTE_EXPORT GETexturedVertex : public GEVertex {
        FVector2D textureCoord;
        GETexturedVertex(GEColoredVertex && vertex);
    };


    typedef std::vector<GEColoredVertex> ColoredVertexVector;
    typedef std::vector<GETexturedVertex> TexturedVertexVector;

    /**
     @brief The Omega Graphics Engine
    */
    class OMEGAGTE_EXPORT OmegaGraphicsEngine {
    public:
        /** 
        @brief Creates an Instance of the Omega Graphics Engine  
        (NEVER CALL THIS FUNCTION! Please invoke GTE::Init())
        @returns SharedHandle<OmegaGraphicsEngine>
        */
        static SharedHandle<OmegaGraphicsEngine> Create();
        /**
        @brief Loads the Omega GTE Shader Library
        @returns SharedHandle<GEFunctionLibrary>
        */
        virtual SharedHandle<GEFunctionLibrary> loadStdShaderLibrary() = 0;
        /**
         @brief Loads an OmegaSL Shader Library,
         @param path Path to a `.shadermap` file.
         @returns SharedHandle<GEFunctionLibrary>
        */
        virtual SharedHandle<GEFunctionLibrary> loadShaderLibrary(FS::Path path) = 0;
        /**
         @brief Creates a GEFence.
         @returns SharedHandle<GEFence>
        */
        virtual SharedHandle<GEFence> makeFence() = 0;

        /**
         @brief Creates a GEBuffer from a BufferDescriptor.
         @param[in] desc The Buffer Descriptor, which could describe a buffer at any length with any object.
         @returns SharedHandle<GEBuffer>
        */
        virtual SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor & desc) = 0;

        /**
         @brief Creates a GETexture from a TextureDescriptor.
         @param[in] desc The Texture Descriptor,  which could describe a 2D, 3D, 2D-Multisampled,or 3D-Multisampled texture with any given width, height (and depth).
         @returns SharedHandle<GETexture>
        */
        virtual SharedHandle<GETexture> makeTexture(const TextureDescriptor & desc) = 0;

        /**
         @brief Creates a GEHeap from a HeapDescriptor.
         @param[in] desc The Heap Descriptor
         @returns SharedHandle<GEHeap>
        */
        virtual SharedHandle<GEHeap> makeHeap(const HeapDescriptor & desc) = 0;
        
        /**
         @brief Creates a GETextureSampler
         */
        

        /**
         @brief Creates a GERenderPipelineState from a RenderPipelineDescriptor.
         @param[in] desc The Render Pipeline Descriptor
         @returns SharedHandle<GERenderPipelineState>
        */
        virtual SharedHandle<GERenderPipelineState> makeRenderPipelineState(RenderPipelineDescriptor & desc) = 0;

        /** 
         @brief Creates a GEComputePipelineState from a ComputePipelineDescriptor.
         @param[in] desc The Compute Pipeline State
         @returns SharedHandle<GEComputePipelineState>
        */
        virtual SharedHandle<GEComputePipelineState> makeComputePipelineState(ComputePipelineDescriptor & desc) = 0;
        virtual SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor & desc) = 0;
        virtual SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor & desc) = 0;
        virtual SharedHandle<GECommandQueue> makeCommandQueue(unsigned maxBufferCount) = 0;
    };


    


    #ifdef TARGET_DIRECTX
    struct OMEGAGTE_EXPORT NativeRenderTargetDescriptor {
        bool isHwnd;
        HWND hwnd;
        unsigned width;
        unsigned height;
    };

    #endif

    #if defined(TARGET_METAL) && defined(__OBJC__)
    struct OMEGAGTE_EXPORT NativeRenderTargetDescriptor {
        CAMetalLayer *metalLayer;
    };
    #endif

    #if defined(TARGET_VULKAN)
    struct OMEGAGTE_EXPORT NativeRenderTargetDescriptor {
        Window w;
    };
    #endif


_NAMESPACE_END_

#endif
