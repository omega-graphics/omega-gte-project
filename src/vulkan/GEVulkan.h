#define VK_USE_PLATFORM_XLIB_KHR


#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "omegaGTE/GE.h"

#ifndef OMEGAGTE_VULKAN_GEVULKAN_H
#define OMEGAGTE_VULKAN_GEVULKAN_H

_NAMESPACE_BEGIN_
    #define VK_RESULT_SUCCEEDED(val) (val == vk::Result::eSuccess)
    class GEVulkanEngine : public OmegaGraphicsEngine {
    public:
        VmaAllocator memAllocator;
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

        SharedHandle<GERenderPipelineState> makeRenderPipelineState(RenderPipelineDescriptor &desc);

        SharedHandle<GEComputePipelineState> makeComputePipelineState(ComputePipelineDescriptor &desc);

        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc);

        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc);

        SharedHandle<GEFunctionLibrary> loadShaderLibrary(FS::Path path);

        SharedHandle<GEFunctionLibrary> loadStdShaderLibrary();

        static SharedHandle<OmegaGraphicsEngine> Create();
    };

    class GEVulkanBuffer : public GEBuffer {
    public: 
        VmaAllocator allocator;
        vk::UniqueBuffer buffer;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;
        void * data() override {
            void *data_ptr;
            vmaMapMemory(allocator,alloc,&data_ptr);
            return data_ptr;
        };
        void unmap(){
            vmaUnmapMemory(allocator,alloc);
        };
        size_t size() override {
            return alloc_info.size;
        };
        GEVulkanBuffer(vk::UniqueBuffer & buffer, VmaAllocator & allocator):buffer(std::move(buffer)){};
    };

    class GEVulkanHeap : public GEHeap {
        // public:
        // GEVulkanHeap(vk::MemoryHeap & heap):heap(heap){};
    };
    
_NAMESPACE_END_

#endif