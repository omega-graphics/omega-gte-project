#define VK_USE_PLATFORM_XLIB_KHR

#include <vulkan/vulkan.hpp>
#include "omegaGTE/GE.h"

#ifndef OMEGAGTE_VULKAN_GEVULKAN_H
#define OMEGAGTE_VULKAN_GEVULKAN_H

_NAMESPACE_BEGIN_
    #define VK_RESULT_SUCCEEDED(val) (val == vk::Result::eSuccess)
    class GEVulkanEngine : public OmegaGraphicsEngine {
    public:
        vk::Instance instance;
        vk::Device device;
        vk::PhysicalDevice physicalDevice;
        unsigned queueFamilyIndex;
        GEVulkanEngine();
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount);
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc);
        SharedHandle<GEFence> makeFence();
        SharedHandle<GEHeap> makeHeap(const HeapDescriptor &desc);
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc);
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor &desc);
        SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor &desc);
        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc);
        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc);
        static SharedHandle<OmegaGraphicsEngine> Create();
    };
    
_NAMESPACE_END_

#endif