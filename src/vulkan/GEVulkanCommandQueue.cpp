#include "GEVulkanCommandQueue.h"
#include "vulkan/vulkan.hpp"

namespace OmegaGE {
    GEVulkanCommandBuffer::GEVulkanCommandBuffer(vk::CommandBuffer & commandBuffer,GEVulkanCommandQueue *parentQueue):commandBuffer(commandBuffer),parentQueue(parentQueue){
        vk::CommandBufferBeginInfo beginInfo;
        // vk::CommandBufferInheritanceInfo inheritanceInfo;
        beginInfo.pInheritanceInfo = nullptr;
        beginInfo.flags = {};
        commandBuffer.begin(beginInfo);
    };

    void GEVulkanCommandBuffer::commitToQueue(){
        commandBuffer.end();
        vk::SubmitInfo submission;
        submission.commandBufferCount = 1;
        submission.pCommandBuffers = &commandBuffer;
        // parentQueue->commandQueue.submit(1,&submission);
    };

   SharedHandle<GECommandBuffer> GEVulkanCommandQueue::getAvailableBuffer(){
       auto res = std::make_shared<GEVulkanCommandBuffer>(commandBuffers[currentBufferIndex],this);
       ++currentBufferIndex;
       return res;
   };
   GEVulkanCommandQueue::GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size):GECommandQueue(size){
       vk::Result res;
       vk::CommandPoolCreateInfo poolCreateInfo;
       poolCreateInfo.queueFamilyIndex = engine->queueFamilyIndex;
       poolCreateInfo.flags = {};
       commandPool = engine->device.createCommandPool(poolCreateInfo);
       vk::CommandBufferAllocateInfo commandBufferCreateInfo;
       commandBufferCreateInfo.commandBufferCount = size;
       commandBufferCreateInfo.commandPool = commandPool;
       commandBufferCreateInfo.level = vk::CommandBufferLevel::ePrimary;
       commandBuffers.reserve(size);
       res = engine->device.allocateCommandBuffers(&commandBufferCreateInfo,commandBuffers.data());
       if(!VK_RESULT_SUCCEEDED(res)){
           exit(1);
       };
       currentBufferIndex = 0;
       vk::DeviceQueueInfo2 queueInfo;
        queueInfo.queueFamilyIndex = engine->queueFamilyIndex;
        queueInfo.queueIndex = 0;
        engine->device.getQueue2(&queueInfo,&commandQueue);

   };
};