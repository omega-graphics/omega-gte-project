#include "GEVulkan.h"

namespace OmegaGE {
    class GEVulkanCommandQueue;

    class GEVulkanCommandBuffer : public GECommandBuffer {
        GEVulkanCommandQueue *parentQueue;
        vk::CommandBuffer & commandBuffer;
    public:
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
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size);
    };
};