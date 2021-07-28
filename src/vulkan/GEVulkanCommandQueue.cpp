#include "GEVulkanCommandQueue.h"
#include "GEVulkanRenderTarget.h"
#include "GEVulkanPipeline.h"
#include "GEVulkan.h"
#include "GEVulkanTexture.h"

_NAMESPACE_BEGIN_
    GEVulkanCommandBuffer::GEVulkanCommandBuffer(vk::CommandBuffer & commandBuffer,GEVulkanCommandQueue *parentQueue):commandBuffer(commandBuffer),parentQueue(parentQueue){
        vk::CommandBufferBeginInfo beginInfo;
        // vk::CommandBufferInheritanceInfo inheritanceInfo;
        beginInfo.pInheritanceInfo = nullptr;
        beginInfo.flags = {};
        commandBuffer.begin(beginInfo);
    };

    void GEVulkanCommandBuffer::startRenderPass(const GERenderPassDescriptor &desc){
        auto nativeTarget = (GEVulkanNativeRenderTarget *)desc.nRenderTarget;
        vk::RenderPassBeginInfo beginInfo;
        beginInfo.framebuffer = nativeTarget->frameBuffer.get();

        vk::ClearValue val;
        val.color.setFloat32({desc.colorAttachment->clearColor.r,desc.colorAttachment->clearColor.g,desc.colorAttachment->clearColor.b,desc.colorAttachment->clearColor.a});
        beginInfo.clearValueCount = 1;
        beginInfo.pClearValues = &val;
        
        commandBuffer.beginRenderPass(&beginInfo,vk::SubpassContents::eInline);
    };

    void GEVulkanCommandBuffer::setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState){
        auto vulkanPipeline = (GEVulkanRenderPipelineState *)pipelineState.get();
        auto state = vulkanPipeline->pipelineInfo;
        
        auto res = parentQueue->engine->device.createGraphicsPipeline(vulkanPipeline->cache.get(),state);

        if(!VK_RESULT_SUCCEEDED(res.result)){
            printf("Failed to Create Graphics Pipeline");
            exit(1);
        };

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,res.value);
    };

    void GEVulkanCommandBuffer::setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned index){
        auto vk_buffer = (GEVulkanBuffer *)buffer.get();
        commandBuffer.bindIndexBuffer(vk_buffer->buffer.get(),index,vk::IndexType::eUint32);
        /// TODO!
    };

    void GEVulkanCommandBuffer::setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned index){
        auto vk_texture = (GEVulkanTexture *)texture.get();
        /// TODO!
    };

    void GEVulkanCommandBuffer::setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned index){
        auto vk_buffer = (GEVulkanBuffer *)buffer.get();
        commandBuffer.bindIndexBuffer(vk_buffer->buffer.get(),index,vk::IndexType::eUint32);
        /// TODO!
    };

    void GEVulkanCommandBuffer::setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned index){
        /// TODO!
    };



    void GEVulkanCommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx){
        vk::PrimitiveTopology topology;

        switch (polygonType) {
            case GERenderTarget::CommandBuffer::Triangle : 
                topology = vk::PrimitiveTopology::eTriangleList;
                break;
            case GERenderTarget::CommandBuffer::TriangleStrip : {
                topology = vk::PrimitiveTopology::eTriangleStrip;
                break;
            }
        }

        commandBuffer.setPrimitiveTopologyEXT(topology);
        commandBuffer.drawIndexed(vertexCount,1,startIdx,0,0);
    };

    void GEVulkanCommandBuffer::setScissorRects(std::vector<GEScissorRect> scissorRects){
        std::vector<vk::Rect2D> vk_rects;
        for(auto & r : scissorRects){
            vk::Rect2D rect;
            rect.offset.x = r.x;
            rect.offset.y = r.y;
            rect.extent.width = r.width;
            rect.extent.height = r.height;
            vk_rects.push_back(rect);
        };


        commandBuffer.setScissor(0,vk_rects);
    };

    void GEVulkanCommandBuffer::setViewports(std::vector<GEViewport> viewports){
        std::vector<vk::Viewport> vk_viewports;
        for(auto & v : viewports){
            vk::Viewport viewport;
            viewport.x = v.x;
            viewport.y = v.y;
            viewport.width = v.width;
            viewport.height = v.height;
            viewport.minDepth = v.nearDepth;
            viewport.maxDepth = v.farDepth;
            vk_viewports.push_back(viewport);
        };

        commandBuffer.setViewport(0,vk_viewports);
    };

    void GEVulkanCommandBuffer::finishRenderPass(){
        commandBuffer.endRenderPass();
    };

    void GEVulkanCommandBuffer::reset(){
        commandBuffer.reset();
    };

    void GEVulkanCommandQueue::submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer){
        auto buffer = (GEVulkanCommandBuffer *)commandBuffer.get();
        buffer->commandBuffer.end();
    };

   SharedHandle<GECommandBuffer> GEVulkanCommandQueue::getAvailableBuffer(){
       auto res = std::make_shared<GEVulkanCommandBuffer>(commandBuffers[currentBufferIndex],this);
       ++currentBufferIndex;
       return res;
   };

   void GEVulkanCommandQueue::commitToGPU(){
       vk::SubmitInfo submission;
       submission.commandBufferCount = commandBuffers.size();
       submission.pCommandBuffers = commandBuffers.data();
       auto res = commandQueue.submit(1,&submission,VK_NULL_HANDLE);
       if(!VK_RESULT_SUCCEEDED(res)){
           printf("Failed to Submit Command Buffers to GPU");
           exit(1);
       };
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
_NAMESPACE_END_