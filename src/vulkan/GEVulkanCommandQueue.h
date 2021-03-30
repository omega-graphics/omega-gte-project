#include "GEVulkan.h"
#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H
#define OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H


_NAMESPACE_BEGIN_
    class GEVulkanCommandQueue;

    class GEVulkanCommandBuffer : public GECommandBuffer {
        GEVulkanCommandQueue *parentQueue;
        vk::CommandBuffer & commandBuffer;
    public:
        void commitToQueue();
        GEVulkanCommandBuffer(vk::CommandBuffer & commandBuffer,GEVulkanCommandQueue *parentQueue);
    };

    class GEVulkanCommandQueue : public GECommandQueue {
        GEVulkanEngine *engine;
        vk::CommandPool commandPool;
        vk::Queue commandQueue;
        std::vector<vk::CommandBuffer> commandBuffers;
        unsigned currentBufferIndex;
        friend class GEVulkanCommandBuffer;
    public:
        void present();
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size);
    };
_NAMESPACE_END_

#endif