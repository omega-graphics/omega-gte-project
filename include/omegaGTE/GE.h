#include "omegasl.h"

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

#ifdef VULKAN_TARGET_WAYLAND
#include <wayland-client.h>
#endif

#ifdef VULKAN_TARGET_X11
#include <X11/Xlib.h>
#endif
#define DEBUG_ENGINE_PREFIX "GEVulkanEngine_Internal"
#endif

#define DEBUG_STREAM(message) std::cout << "[" << DEBUG_ENGINE_PREFIX << "] - " << message << std::endl

#include "GTEBase.h"

#ifndef OMEGAGTE_GE_H
#define OMEGAGTE_GE_H

_NAMESPACE_BEGIN_
    struct GTE;
    typedef enum : uint8_t {
        Shared,
        GPUOnly
    } StorageOpts;

    struct GTEShader;
    struct GTEShaderLibrary;
    struct GTEDevice;
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

    /// @brief Describes a Texture Render Target
    struct TextureRenderTargetDescriptor {
        bool renderToExistingTexture = false;
        SharedHandle<GETexture> texture = nullptr;
        GRect rect;
    };

    /// @brief A 3D Space sized to fixed dimensions.
    struct OMEGAGTE_EXPORT GEViewport {
        float x,y;
        float width,height;
        float nearDepth,farDepth;
    };

    /// @brief A Cropping Rectangle that clips the GEViewport.
    struct OMEGAGTE_EXPORT GEScissorRect {
        float x,y;
        float width,height;
    };

    /// @brief Describes a Buffer.
    /// @paragraph Each object in the buffer MUST be the identical.
    struct  OMEGAGTE_EXPORT BufferDescriptor {
        /// Describes the usage of the Buffer.
        typedef enum : int {
            Upload,
            Readback,
            GPUOnly
        } Usage;
        /// @enum Usage
        Usage usage = Upload;
        /// The length of the buffer (in bytes).
        size_t len;
        /// The stride of each object in the buffer (in bytes).
        size_t objectStride;
        /// The storage options of the resource.
        StorageOpts opts;
    };
    /// @brief A GPU Buffer Resource
    class  OMEGAGTE_EXPORT GEBuffer {
    protected:
        BufferDescriptor::Usage usage;
        bool checkCanWrite();
        bool checkCanRead();
        explicit GEBuffer(const BufferDescriptor::Usage & usage);
    public:
        virtual size_t size() = 0;
        virtual ~GEBuffer() = default;
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

    /// @brief Provides command synchronization across multiple command queues.
    class  OMEGAGTE_EXPORT GEFence {};

    /// @brief Describes a Texture Sampler
    struct OMEGAGTE_EXPORT SamplerDescriptor {
        enum class AddressMode  : int {
            Wrap,
            ClampToEdge,
            MirrorClampToEdge,
            MirrorWrap,
        }
        /// @brief  Address Mode for Width
        uAddressMode = AddressMode::Wrap,
        /// @brief Address Mode for Height
        vAddressMode = AddressMode::Wrap,
        /// @brief Address Mode for Depth
        wAddressMode = AddressMode::Wrap;
        enum class Filter : int {
            Linear,
            Point,
            MagLinearMinPointMipLinear,
            MagPointMinLinearMipLinear,
            MagLinearMinLinearMipPoint,
            MagPointMinLinearMipPoint,
            MagLinearMinPointMipPoint,
            MagPointMinPointMipLinear,
            MaxAnisotropic,
            MinAnisotropic
        } filter = Filter::Linear;
        unsigned int maxAnisotropy = 16;

    };

    class OMEGAGTE_EXPORT GESamplerState {};

    /**
     @brief The Omega Graphics Engine
    */
    class OMEGAGTE_EXPORT OmegaGraphicsEngine {
        SharedHandle<GTEShaderLibrary> loadShaderLibraryFromInputStream(std::istream & in);
    protected:
        virtual SharedHandle<GTEShader> _loadShaderFromDesc(omegasl_shader *shaderDesc) = 0;
    public:
        /** 
        @brief Creates an Instance of the Omega Graphics Engine  
        (NEVER CALL THIS FUNCTION! Please invoke GTE::Init())
        @returns SharedHandle<OmegaGraphicsEngine>
        */
        static SharedHandle<OmegaGraphicsEngine> Create(SharedHandle<GTEDevice> & device);
         /**
          @brief Loads an OmegaSL Shader Library,
          @param path Path to an `omegasllib` file.
          @returns SharedHandle<GEShaderLibrary>
         */
         SharedHandle<GTEShaderLibrary> loadShaderLibrary(FS::Path path);
        /**
         @brief Creates a GEFence.
         @returns SharedHandle<GEFence>
         @see GEFence
        */
        virtual SharedHandle<GEFence> makeFence() = 0;

        /**
         @brief Creates a GESamplerState
         @returns SharedHandle<GESamplerState>
         * */
         virtual SharedHandle<GESamplerState> makeSamplerState(const SamplerDescriptor &desc) = 0;

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
    #ifdef VULKAN_TARGET_X11
        Window x_window;
        Display *x_display;
    #endif
    #ifdef VULKAN_TARGET_WAYLAND
        wl_surface *wl_surface = nullptr;
        wl_display *wl_display = nullptr;
        unsigned width;
        unsigned height;
    #endif
    #ifdef VULKAN_TARGET_ANDROID
    #endif
    };
    #endif


_NAMESPACE_END_

#endif
