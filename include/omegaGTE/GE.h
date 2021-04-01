#if defined(TARGET_DIRECTX)
#include <windows.h>
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
    typedef enum : uint8_t {
        Private,
        Managed,
        Shared
    } StorageOpts;

    typedef struct __GEFunctionInternal GEFunction;
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

    struct GEViewport {

    };

    struct BufferDescriptor {
        size_t len;
        StorageOpts opts;
    };

    class GEBuffer {

    };

    struct HeapDescriptor {

    };

    class GEHeap {

    };

    class GEFence {

    };

    /**
     The Omega GE Main Class
    */
    class OMEGAGTE_EXPORT OmegaGraphicsEngine {
    public:
        /**
         Creates an Instance of the Omega Graphics Engine
        @returns SharedHandle<OmegaGraphicsEngine>
        */
        static SharedHandle<OmegaGraphicsEngine> Create();

        /**
          Creates a GEFence.
         @returns SharedHandle<GEFence>
        */
        virtual SharedHandle<GEFence> makeFence() = 0;

        /**
         Creates a GEBuffer from a BufferDescriptor.
         @param[in] desc The Buffer Descriptor, which could describe a buffer at any length with any object.
         @returns SharedHandle<GEBuffer>
        */
        virtual SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor & desc) = 0;

        /**
          Creates a GETexture from a TextureDescriptor.
         @param[in] desc The Texture Descriptor,  which could describe a 2D, 3D, 2D-Multisampled,or 3D-Multisampled texture with any given width, height (and depth).
         @returns SharedHandle<GETexture>
        */
        virtual SharedHandle<GETexture> makeTexture(const TextureDescriptor & desc) = 0;

        /**
          Creates a GEHeap from a HeapDescriptor.
         @param[in] desc The Heap Descriptor
         @returns SharedHandle<GEHeap>
        */
        virtual SharedHandle<GEHeap> makeHeap(const HeapDescriptor & desc) = 0;

        /**
         Creates a GERenderPipelineState from a RenderPipelineDescriptor.
         @param[in] desc The Render Pipeline Descriptor
         @returns SharedHandle<GERenderPipelineState>
        */
        virtual SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor & desc) = 0;

        /** 
         Creates a GEComputePipelineState from a ComputePipelineDescriptor.
         @param[in] desc The Compute Pipeline State
         @returns SharedHandle<GEComputePipelineState>
        */
        virtual SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor & desc) = 0;
        virtual SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor & desc) = 0;
        virtual SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor & desc) = 0;
        virtual SharedHandle<GECommandQueue> makeCommandQueue(unsigned maxBufferCount) = 0;
    };



    #ifdef TARGET_DIRECTX
    struct OMEGAGTE_EXPORT NativeRenderTargetDescriptor {
        HWND hwnd;
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
