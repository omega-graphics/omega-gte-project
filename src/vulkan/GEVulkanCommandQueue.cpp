#include "GEVulkanCommandQueue.h"
#include "GEVulkanRenderTarget.h"
#include "GEVulkanPipeline.h"
#include "GEVulkan.h"
#include "GEVulkanTexture.h"

_NAMESPACE_BEGIN_
    unsigned int GEVulkanCommandBuffer::getBindingForResourceID(unsigned int &id, omegasl_shader &shader) {\
        ArrayRef<omegasl_shader_layout_desc> layoutDesc {shader.pLayout,shader.pLayout + shader.nLayout};
        for(auto & l : layoutDesc){
            if(l.location == id){
                return l.gpu_relative_loc;
            }
        }
        return 0;
    }

    unsigned int GEVulkanCommandBuffer::getDescriptorSetIndexForResourceID(unsigned int &id) {
        if(renderPipelineState != nullptr){
            auto desc = renderPipelineState->descMap[id];
            unsigned idx = 0;
            for(;idx != renderPipelineState->descs.size();idx++){
                if(renderPipelineState->descs[idx] == desc){
                    break;
                }
            }
            return idx;
        }
        else {

        }
    }

    GEVulkanCommandBuffer::GEVulkanCommandBuffer(VkCommandBuffer & commandBuffer,GEVulkanCommandQueue *parentQueue):commandBuffer(commandBuffer),parentQueue(parentQueue){
        VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        // vk::CommandBufferInheritanceInfo inheritanceInfo;
        beginInfo.pInheritanceInfo = nullptr;
        beginInfo.flags = 0;
        vkBeginCommandBuffer(commandBuffer,&beginInfo);
    };

    void GEVulkanCommandBuffer::startRenderPass(const GERenderPassDescriptor &desc){
        auto nativeTarget = (GEVulkanNativeRenderTarget *)desc.nRenderTarget;
        VkRenderPassBeginInfo beginInfo {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        beginInfo.framebuffer = nativeTarget->frameBuffer;

        VkClearValue val;
        val.color.float32[0] = desc.colorAttachment->clearColor.r;
        val.color.float32[1] = desc.colorAttachment->clearColor.g;
        val.color.float32[2] = desc.colorAttachment->clearColor.b;
        val.color.float32[3] = desc.colorAttachment->clearColor.a;

        beginInfo.clearValueCount = 1;
        beginInfo.pClearValues = &val;

        vkCmdBeginRenderPass(commandBuffer,&beginInfo,VK_SUBPASS_CONTENTS_INLINE);
    };

    void GEVulkanCommandBuffer::setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState){
        auto vulkanPipeline = (GEVulkanRenderPipelineState *)pipelineState.get();
        VkPipeline state = vulkanPipeline->pipeline;

        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,state);
        renderPipelineState = vulkanPipeline;
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vulkanPipeline->layout,
                                0,
                                vulkanPipeline->descs.size(),
                                vulkanPipeline->descs.data(),
                                0,nullptr);
    };

    void GEVulkanCommandBuffer::bindResourceAtVertexShader(SharedHandle<GEBuffer> &buffer, unsigned id){
        auto vk_buffer = (GEVulkanBuffer *)buffer.get();

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,renderPipelineState->vertexShader->internal);
        writeInfo.descriptorCount = 1;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        writeInfo.pNext = nullptr;
        writeInfo.dstArrayElement = 0;
        writeInfo.pBufferInfo = nullptr;
        writeInfo.pImageInfo = nullptr;
        writeInfo.pTexelBufferView = &vk_buffer->bufferView;

        vkCmdPushDescriptorSetKHR(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,renderPipelineState->layout,
                                  getDescriptorSetIndexForResourceID(id),1,&writeInfo);
    };

    void GEVulkanCommandBuffer::bindResourceAtVertexShader(SharedHandle<GETexture> &texture, unsigned id){
        auto vk_texture = (GEVulkanTexture *)texture.get();
        /// TODO!

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,renderPipelineState->vertexShader->internal);
        writeInfo.descriptorCount = 1;

        VkDescriptorImageInfo imgInfo {};
        imgInfo.sampler = VK_NULL_HANDLE;
        imgInfo.imageView = vk_texture->img_view;

        VkDescriptorType t;

        if(vk_texture->memoryUsage == VMA_MEMORY_USAGE_CPU_TO_GPU){
            t = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
        else {
            t = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }

        writeInfo.descriptorType = t;
        writeInfo.pNext = nullptr;
        writeInfo.dstArrayElement = 0;
        writeInfo.pBufferInfo = nullptr;
        writeInfo.pImageInfo = &imgInfo;

        vkCmdPushDescriptorSetKHR(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,renderPipelineState->layout,
                                  getDescriptorSetIndexForResourceID(id),1,&writeInfo);
    };

    void GEVulkanCommandBuffer::bindResourceAtFragmentShader(SharedHandle<GEBuffer> &buffer, unsigned id){
        auto vk_buffer = (GEVulkanBuffer *)buffer.get();

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,renderPipelineState->fragmentShader->internal);
        writeInfo.descriptorCount = 1;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        writeInfo.pNext = nullptr;
        writeInfo.dstArrayElement = 0;
        writeInfo.pBufferInfo = nullptr;
        writeInfo.pImageInfo = nullptr;
        writeInfo.pTexelBufferView = &vk_buffer->bufferView;

        vkCmdPushDescriptorSetKHR(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,renderPipelineState->layout,
                                  getDescriptorSetIndexForResourceID(id),1,&writeInfo);
    };

    void GEVulkanCommandBuffer::bindResourceAtFragmentShader(SharedHandle<GETexture> &texture, unsigned id){
        /// TODO!

        auto vk_texture = (GEVulkanTexture *)texture.get();
        /// TODO!

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,renderPipelineState->fragmentShader->internal);
        writeInfo.descriptorCount = 1;

        VkDescriptorImageInfo imgInfo {};
        imgInfo.sampler = VK_NULL_HANDLE;
        imgInfo.imageView = vk_texture->img_view;

        VkDescriptorType t;

        if(vk_texture->memoryUsage == VMA_MEMORY_USAGE_CPU_TO_GPU){
            t = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
        else {
            t = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }

        writeInfo.descriptorType = t;
        writeInfo.pNext = nullptr;
        writeInfo.dstArrayElement = 0;
        writeInfo.pBufferInfo = nullptr;
        writeInfo.pImageInfo = &imgInfo;

        vkCmdPushDescriptorSetKHR(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,renderPipelineState->layout,
                                  getDescriptorSetIndexForResourceID(id),1,&writeInfo);
    };



    void GEVulkanCommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType, unsigned int vertexCount, size_t startIdx){
        VkPrimitiveTopology topology;

        switch (polygonType) {
            case GERenderTarget::CommandBuffer::Triangle : 
                topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                break;
            case GERenderTarget::CommandBuffer::TriangleStrip : {
                topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                break;
            }
        }

        vkCmdSetPrimitiveTopologyEXT(commandBuffer,topology);
        vkCmdDraw(commandBuffer,vertexCount,1,startIdx,0);
    };

    void GEVulkanCommandBuffer::setScissorRects(std::vector<GEScissorRect> scissorRects){
        std::vector<VkRect2D> vk_rects;
        for(auto & r : scissorRects){
            VkRect2D rect {};
            rect.offset.x = r.x;
            rect.offset.y = r.y;
            rect.extent.width = r.width;
            rect.extent.height = r.height;
            vk_rects.push_back(rect);
        };

        vkCmdSetScissor(commandBuffer,0,vk_rects.size(),vk_rects.data());
    };

    void GEVulkanCommandBuffer::setViewports(std::vector<GEViewport> viewports){
        std::vector<VkViewport> vk_viewports;
        for(auto & v : viewports){
            VkViewport viewport {};
            viewport.x = v.x;
            viewport.y = v.y;
            viewport.width = v.width;
            viewport.height = v.height;
            viewport.minDepth = v.nearDepth;
            viewport.maxDepth = v.farDepth;
            vk_viewports.push_back(viewport);
        };

        vkCmdSetViewport(commandBuffer,0,vk_viewports.size(),vk_viewports.data());
    };

    void GEVulkanCommandBuffer::finishRenderPass(){
        vkCmdEndRenderPass(commandBuffer);
        renderPipelineState = nullptr;
    };

    void GEVulkanCommandBuffer::copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest) {
        auto src_img = (GEVulkanTexture *)src.get(),dest_img = (GEVulkanTexture *)dest.get();
        VkImageCopy imgCopy {};
        imgCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgCopy.srcSubresource.baseArrayLayer = 0;
        imgCopy.srcSubresource.layerCount = 1;
        imgCopy.srcSubresource.mipLevel = src_img->descriptor.mipLevels;
        imgCopy.srcOffset = {0,0,0};
        imgCopy.dstOffset = {0,0,0};
        imgCopy.dstSubresource.mipLevel = dest_img->descriptor.mipLevels;
        imgCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgCopy.dstSubresource.layerCount = 1;
        imgCopy.dstSubresource.baseArrayLayer = 0;
        imgCopy.extent = {src_img->descriptor.width,src_img->descriptor.height,src_img->descriptor.depth};
        vkCmdCopyImage(commandBuffer,src_img->img,src_img->layout,dest_img->img,dest_img->layout,1,&imgCopy);
    }

    void GEVulkanCommandBuffer::copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest,
                                                     const TextureRegion &region, const GPoint3D &destCoord) {
        auto src_img = (GEVulkanTexture *)src.get(),dest_img = (GEVulkanTexture *)dest.get();
        VkImageCopy imgCopy {};
        imgCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgCopy.srcSubresource.baseArrayLayer = 0;
        imgCopy.srcSubresource.layerCount = 1;
        imgCopy.srcSubresource.mipLevel = src_img->descriptor.mipLevels;
        imgCopy.srcOffset = {int32_t(region.x),int32_t(region.y),int32_t(region.z)};
        imgCopy.dstOffset = {int32_t(destCoord.x),int32_t(destCoord.y),int32_t(destCoord.z)};
        imgCopy.dstSubresource.mipLevel = dest_img->descriptor.mipLevels;
        imgCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgCopy.dstSubresource.layerCount = 1;
        imgCopy.dstSubresource.baseArrayLayer = 0;
        imgCopy.extent = {region.w,region.h,region.d};
        vkCmdCopyImage(commandBuffer,src_img->img,src_img->layout,dest_img->img,dest_img->layout,1,&imgCopy);
    }

    void GEVulkanCommandBuffer::reset(){
        vkResetCommandBuffer(commandBuffer,VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    };

    void GEVulkanCommandQueue::submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer){
        auto buffer = (GEVulkanCommandBuffer *)commandBuffer.get();
        vkEndCommandBuffer(buffer->commandBuffer);
    };

   SharedHandle<GECommandBuffer> GEVulkanCommandQueue::getAvailableBuffer(){
       auto res = std::make_shared<GEVulkanCommandBuffer>(commandBuffers[currentBufferIndex],this);
       ++currentBufferIndex;
       return res;
   };

   void GEVulkanCommandQueue::commitToGPU(){
       VkSubmitInfo submission {VK_STRUCTURE_TYPE_SUBMIT_INFO};
       submission.commandBufferCount = commandBuffers.size();
       submission.pCommandBuffers = commandBuffers.data();
       submission.pNext = nullptr;
       VkFence fence;
       auto res = vkQueueSubmit(commandQueue,1,&submission,fence);
       if(!VK_RESULT_SUCCEEDED(res)){
           printf("Failed to Submit Command Buffers to GPU");
           exit(1);
       };
   };
   
   GEVulkanCommandQueue::GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size):GECommandQueue(size){
       VkResult res;
       VkCommandPoolCreateInfo poolCreateInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};

       unsigned queueFamilyIndex = 0;
       bool foundQueueFamily = false;
       for(auto & qf : engine->queueFamilyProps){
           if(qf.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)){
               foundQueueFamily = true;
               break;
           }
           ++queueFamilyIndex;
       }

       if(!foundQueueFamily){
           std::cout << "Failed to find queue family" << std::endl;
           exit(1);
       }

       poolCreateInfo.queueFamilyIndex = queueFamilyIndex;
       poolCreateInfo.pNext = nullptr;
       poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

       res = vkCreateCommandPool(engine->device,&poolCreateInfo,nullptr,&commandPool);

       if(!VK_RESULT_SUCCEEDED(res)){
           exit(1);
       };

       VkCommandBufferAllocateInfo commandBufferCreateInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
       commandBufferCreateInfo.commandBufferCount = size;
       commandBufferCreateInfo.commandPool = commandPool;
       commandBufferCreateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
       commandBufferCreateInfo.pNext = nullptr;
       commandBuffers.reserve(size);

       res = vkAllocateCommandBuffers(engine->device,&commandBufferCreateInfo,commandBuffers.data());

       if(!VK_RESULT_SUCCEEDED(res)){
           exit(1);
       };
       
       currentBufferIndex = 0;

   };

   GEVulkanCommandQueue::~GEVulkanCommandQueue() {
       vkFreeCommandBuffers(engine->device,commandPool,commandBuffers.size(),commandBuffers.data());
       commandBuffers.resize(0);
       vkDestroyCommandPool(engine->device,commandPool,nullptr);
   }
_NAMESPACE_END_