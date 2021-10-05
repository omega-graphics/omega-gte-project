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
            auto desc = computePipelineState->descMap[id];
            unsigned idx = 0;
            for(;idx != computePipelineState->descs.size();idx++){
                if(computePipelineState->descs[idx] == desc){
                    break;
                }
            }
            return idx;
        }
    }

    omegasl_shader_layout_desc_io_mode
    GEVulkanCommandBuffer::getResourceIOModeForResourceID(unsigned int &id, omegasl_shader &shader) {
        ArrayRef<omegasl_shader_layout_desc> layoutDesc {shader.pLayout,shader.pLayout + shader.nLayout};
        for(auto & l : layoutDesc){
            if(l.location == id){
                return l.io_mode;
            }
        }
        return OMEGASL_SHADER_DESC_IO_INOUT;
    }

    void GEVulkanCommandBuffer::insertResourceBarrierIfNeeded(GEVulkanBuffer *buffer, unsigned int &resource_id,
                                                              omegasl_shader &shader) {
        auto ioMode = getResourceIOModeForResourceID(resource_id,shader);

        VkAccessFlags2KHR shaderAccess;
        VkPipelineStageFlags2KHR pipelineStage;

        if(shader.type == OMEGASL_SHADER_VERTEX){
            pipelineStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR;
        }
        else if(shader.type == OMEGASL_SHADER_FRAGMENT){
            pipelineStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
        }
        else {
            pipelineStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR;
        }

        bool hasPipelineAccess = buffer->priorPipelineAccess != 0;

        if(ioMode == OMEGASL_SHADER_DESC_IO_IN){
            shaderAccess = VK_ACCESS_2_SHADER_READ_BIT_KHR;
        }
        else if(ioMode == OMEGASL_SHADER_DESC_IO_INOUT){
            shaderAccess = VK_ACCESS_2_SHADER_WRITE_BIT_KHR | VK_ACCESS_2_SHADER_READ_BIT_KHR;
        }
        else {
            shaderAccess = VK_ACCESS_2_SHADER_WRITE_BIT_KHR;
        }

        if(buffer->priorAccess != 0 && hasPipelineAccess){
            VkBufferMemoryBarrier2KHR bufferMemoryBarrier2Khr {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR};
            bufferMemoryBarrier2Khr.srcQueueFamilyIndex = bufferMemoryBarrier2Khr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferMemoryBarrier2Khr.buffer = buffer->buffer;
            bufferMemoryBarrier2Khr.offset = buffer->alloc_info.offset;
            bufferMemoryBarrier2Khr.size = buffer->alloc_info.size;
            bufferMemoryBarrier2Khr.srcAccessMask = buffer->priorAccess;
            bufferMemoryBarrier2Khr.dstAccessMask = shaderAccess;
            bufferMemoryBarrier2Khr.srcStageMask = buffer->priorPipelineAccess;
            bufferMemoryBarrier2Khr.dstStageMask = pipelineStage;
            bufferMemoryBarrier2Khr.pNext = nullptr;

            VkDependencyInfoKHR dependencyInfoKhr {VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR};
            dependencyInfoKhr.pNext = nullptr;
            dependencyInfoKhr.bufferMemoryBarrierCount = 1;
            dependencyInfoKhr.pBufferMemoryBarriers = &bufferMemoryBarrier2Khr;
            vkCmdPipelineBarrier2KHR(commandBuffer,&dependencyInfoKhr);
        }

        buffer->priorPipelineAccess = pipelineStage;
        buffer->priorAccess = shaderAccess;
    }

    void GEVulkanCommandBuffer::insertResourceBarrierIfNeeded(GEVulkanTexture *texture, unsigned int &resource_id,
                                                              omegasl_shader &shader) {

        auto ioMode = getResourceIOModeForResourceID(resource_id,shader);

        /// Use Pipeline Barrier if Access changes
        VkAccessFlags2KHR shaderAccess;
        VkImageLayout layout;
        VkPipelineStageFlags2KHR pipelineStage;

        if(shader.type == OMEGASL_SHADER_VERTEX){
            pipelineStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR;
        }
        else if(shader.type == OMEGASL_SHADER_FRAGMENT){
            pipelineStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
        }
        else {
            pipelineStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR;
        }

        bool hasPipelineAccess = texture->priorPipelineAccess != 0;

        if(ioMode == OMEGASL_SHADER_DESC_IO_IN){
            shaderAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR;
            layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        }
        else if(ioMode == OMEGASL_SHADER_DESC_IO_INOUT){
            shaderAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR;
            layout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else {
            shaderAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT_KHR;
            layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }
        /// If not first time access, pipeline barrier must be inserted before binding.
        if(texture->priorShaderAccess != 0 && hasPipelineAccess){
            VkImageMemoryBarrier2KHR imageMemoryBarrier2Khr {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR};
            imageMemoryBarrier2Khr.pNext = nullptr;
            imageMemoryBarrier2Khr.srcAccessMask = texture->priorShaderAccess;
            imageMemoryBarrier2Khr.dstAccessMask = shaderAccess;
            imageMemoryBarrier2Khr.image = texture->img;
            imageMemoryBarrier2Khr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier2Khr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier2Khr.oldLayout = texture->layout;
            imageMemoryBarrier2Khr.newLayout = layout;
            imageMemoryBarrier2Khr.srcStageMask = texture->priorPipelineAccess;
            imageMemoryBarrier2Khr.dstStageMask = pipelineStage;


            VkDependencyInfoKHR dependencyInfoKhr {VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR};
            dependencyInfoKhr.pNext = nullptr;
            dependencyInfoKhr.imageMemoryBarrierCount = 1;
            dependencyInfoKhr.pImageMemoryBarriers = &imageMemoryBarrier2Khr;
            vkCmdPipelineBarrier2KHR(commandBuffer,&dependencyInfoKhr);
        }

        texture->layout = layout;
        texture->priorShaderAccess = shaderAccess;
        texture->priorPipelineAccess = pipelineStage;
}

    GEVulkanCommandBuffer::GEVulkanCommandBuffer(VkCommandBuffer & commandBuffer,GEVulkanCommandQueue *parentQueue):commandBuffer(commandBuffer),parentQueue(parentQueue){
        VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        // vk::CommandBufferInheritanceInfo inheritanceInfo;
        beginInfo.pInheritanceInfo = nullptr;
        beginInfo.flags = 0;
        vkBeginCommandBuffer(commandBuffer,&beginInfo);
    };

    void GEVulkanCommandBuffer::startRenderPass(const GERenderPassDescriptor &desc){
        VkRenderPassBeginInfo beginInfo {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};

        VkRenderPass renderPass;

        VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};

        if(desc.nRenderTarget != nullptr) {
            auto nativeTarget = (GEVulkanNativeRenderTarget *) desc.nRenderTarget;

            vkWaitForFences(parentQueue->engine->device,1,&nativeTarget->frameIsReadyFence,VK_TRUE,UINT64_MAX);
            vkResetFences(parentQueue->engine->device,1,&nativeTarget->frameIsReadyFence);

            VkAttachmentDescription attachmentDescription{};
            attachmentDescription.format = nativeTarget->format;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;


            switch (desc.colorAttachment->loadAction) {
                case GERenderTarget::RenderPassDesc::ColorAttachment::Load : {
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
                case GERenderTarget::RenderPassDesc::ColorAttachment::LoadPreserve : {
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                }
                case GERenderTarget::RenderPassDesc::ColorAttachment::Discard : {
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
            }

            VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

            VkSubpassDescription subpass = {0};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &color_ref;

            VkSubpassDependency dependency = {0};
            dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass          = 0;
            dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;


            dependency.srcAccessMask = 0;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            renderPassCreateInfo.attachmentCount = 1;
            renderPassCreateInfo.pAttachments = &attachmentDescription;
            renderPassCreateInfo.dependencyCount = 1;
            renderPassCreateInfo.pDependencies = &dependency;
            renderPassCreateInfo.subpassCount = 1;
            renderPassCreateInfo.pSubpasses = &subpass;

            vkCreateRenderPass(parentQueue->engine->device,&renderPassCreateInfo,nullptr,&renderPass);

            beginInfo.framebuffer = nativeTarget->framebuffer;
            beginInfo.renderArea.extent = nativeTarget->extent;
        }
        else {
            auto textureTarget = (GEVulkanTextureRenderTarget *)desc.tRenderTarget;
            vkCreateRenderPass(parentQueue->engine->device,&renderPassCreateInfo,nullptr,&renderPass);

            VkAttachmentDescription attachmentDescription{};
            attachmentDescription.format = textureTarget->texture->format;
            attachmentDescription.initialLayout = textureTarget->texture->layout;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
            attachmentDescription.samples = VkSampleCountFlagBits(textureTarget->texture->descriptor.sampleCount);

            textureTarget->texture->layout = VK_IMAGE_LAYOUT_GENERAL;


            switch (desc.colorAttachment->loadAction) {
                case GERenderTarget::RenderPassDesc::ColorAttachment::Clear : {
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                }
                case GERenderTarget::RenderPassDesc::ColorAttachment::Load : {
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
                case GERenderTarget::RenderPassDesc::ColorAttachment::LoadPreserve : {
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                }
                case GERenderTarget::RenderPassDesc::ColorAttachment::Discard : {
                    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                }
            }

            VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

            VkSubpassDescription subpass = {0};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &color_ref;

            VkSubpassDependency dependency = {0};
            dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass          = 0;
            dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;


            dependency.srcAccessMask = 0;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            renderPassCreateInfo.attachmentCount = 1;
            renderPassCreateInfo.pAttachments = &attachmentDescription;
            renderPassCreateInfo.dependencyCount = 1;
            renderPassCreateInfo.pDependencies = &dependency;
            renderPassCreateInfo.subpassCount = 1;
            renderPassCreateInfo.pSubpasses = &subpass;

            beginInfo.framebuffer = textureTarget->frameBuffer;
            beginInfo.renderArea.extent = {textureTarget->texture->descriptor.width,textureTarget->texture->descriptor.height};
        }



        VkClearValue val;
        val.color.float32[0] = desc.colorAttachment->clearColor.r;
        val.color.float32[1] = desc.colorAttachment->clearColor.g;
        val.color.float32[2] = desc.colorAttachment->clearColor.b;
        val.color.float32[3] = desc.colorAttachment->clearColor.a;

        beginInfo.clearValueCount = 1;
        beginInfo.pClearValues = &val;
        beginInfo.renderPass = renderPass;
        beginInfo.renderArea.offset.x = 0;
        beginInfo.renderArea.offset.y = 0;

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

        insertResourceBarrierIfNeeded(vk_buffer,id,renderPipelineState->vertexShader->internal);

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

        auto ioMode = getResourceIOModeForResourceID(id,renderPipelineState->vertexShader->internal);

        insertResourceBarrierIfNeeded(vk_texture,id,renderPipelineState->vertexShader->internal);

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,renderPipelineState->vertexShader->internal);
        writeInfo.descriptorCount = 1;

        VkDescriptorImageInfo imgInfo {};
        imgInfo.sampler = VK_NULL_HANDLE;
        imgInfo.imageView = vk_texture->img_view;
        imgInfo.imageLayout = vk_texture->layout;

        VkDescriptorType t;


        if(ioMode == OMEGASL_SHADER_DESC_IO_IN){
            t = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
        else {
            t=  VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
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

        insertResourceBarrierIfNeeded(vk_buffer,id,renderPipelineState->fragmentShader->internal);

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

        auto vk_texture = (GEVulkanTexture *)texture.get();

        auto ioMode = getResourceIOModeForResourceID(id,renderPipelineState->fragmentShader->internal);

        insertResourceBarrierIfNeeded(vk_texture,id,renderPipelineState->fragmentShader->internal);

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,renderPipelineState->fragmentShader->internal);
        writeInfo.descriptorCount = 1;

        VkDescriptorImageInfo imgInfo {};
        imgInfo.sampler = VK_NULL_HANDLE;
        imgInfo.imageView = vk_texture->img_view;
        imgInfo.imageLayout = vk_texture->layout;

        VkDescriptorType t;

        if(ioMode == OMEGASL_SHADER_DESC_IO_IN){
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

    void GEVulkanCommandBuffer::setVertexBuffer(SharedHandle<GEBuffer> &buffer) {
        auto vkBuffer = ((GEVulkanBuffer *)buffer.get());
        vkCmdBindVertexBuffers(commandBuffer,0,4,&vkBuffer->buffer,nullptr);
    }

    void GEVulkanCommandBuffer::finishRenderPass(){
        vkCmdEndRenderPass(commandBuffer);
        renderPipelineState = nullptr;
    };

    void GEVulkanCommandBuffer::startComputePass(const GEComputePassDescriptor &desc) {
        inComputePass = true;
    }

    void GEVulkanCommandBuffer::setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState) {
        auto *vkPipelineState = (GEVulkanComputePipelineState *)pipelineState.get();
        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,vkPipelineState->pipeline);

        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                vkPipelineState->layout,
                                0,
                                vkPipelineState->descs.size(),
                                vkPipelineState->descs.data(),
                                0,
                                nullptr);
        computePipelineState = vkPipelineState;
    }

    void GEVulkanCommandBuffer::bindResourceAtComputeShader(SharedHandle<GEBuffer> &buffer, unsigned int id) {
        auto vk_buffer = (GEVulkanBuffer *)buffer.get();

        insertResourceBarrierIfNeeded(vk_buffer,id,computePipelineState->computeShader->internal);

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,computePipelineState->computeShader->internal);
        writeInfo.descriptorCount = 1;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        writeInfo.pNext = nullptr;
        writeInfo.dstArrayElement = 0;
        writeInfo.pBufferInfo = nullptr;
        writeInfo.pImageInfo = nullptr;
        writeInfo.pTexelBufferView = &vk_buffer->bufferView;

        vkCmdPushDescriptorSetKHR(commandBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,computePipelineState->layout,
                                  getDescriptorSetIndexForResourceID(id),1,&writeInfo);
    }

    void GEVulkanCommandBuffer::bindResourceAtComputeShader(SharedHandle<GETexture> &texture, unsigned int id) {

        auto vk_texture = (GEVulkanTexture *)texture.get();

        insertResourceBarrierIfNeeded(vk_texture,id,computePipelineState->computeShader->internal);

        VkWriteDescriptorSet writeInfo {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeInfo.dstBinding = getBindingForResourceID(id,computePipelineState->computeShader->internal);
        writeInfo.descriptorCount = 1;

        VkDescriptorImageInfo imgInfo {};
        imgInfo.sampler = VK_NULL_HANDLE;
        imgInfo.imageView = vk_texture->img_view;
        imgInfo.imageLayout = vk_texture->layout;

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

        vkCmdPushDescriptorSetKHR(commandBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,computePipelineState->layout,
                                  getDescriptorSetIndexForResourceID(id),1,&writeInfo);
    }

    void GEVulkanCommandBuffer::dispatchThreads(unsigned int x, unsigned int y, unsigned int z) {
        vkCmdDispatch(commandBuffer,x,y,z);
    }

    void GEVulkanCommandBuffer::finishComputePass() {
        inComputePass = false;
    }

    void GEVulkanCommandBuffer::startBlitPass() {
        inBlitPass = true;
    }

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

    void GEVulkanCommandBuffer::finishBlitPass() {
        inBlitPass = false;
    }

    void GEVulkanCommandBuffer::reset(){
        vkResetCommandBuffer(commandBuffer,VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    };

    void GEVulkanCommandBuffer::signalFence(SharedHandle<GEFence> &fence, unsigned int val) {
        auto vkFence = ((GEVulkanFence *)fence.get());
        vkCmdSetEvent(commandBuffer,vkFence->event,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }

    void GEVulkanCommandBuffer::waitForFence(SharedHandle<GEFence> &fence, unsigned int val) {
        auto vkFence = ((GEVulkanFence *)fence.get());
        vkCmdWaitEvents(commandBuffer,
                        1,
                        &vkFence->event,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        0,
                        nullptr,0,nullptr,0,nullptr);
        vkCmdResetEvent(commandBuffer,vkFence->event,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }

    void GEVulkanCommandQueue::submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer){
        auto buffer = (GEVulkanCommandBuffer *)commandBuffer.get();
        commandQueue.push_back(buffer->commandBuffer);
    };

   SharedHandle<GECommandBuffer> GEVulkanCommandQueue::getAvailableBuffer(){
       auto res = std::make_shared<GEVulkanCommandBuffer>(commandBuffers[currentBufferIndex],this);
       ++currentBufferIndex;
       return res;
   };

    VkCommandBuffer &GEVulkanCommandQueue::getLastCommandBufferInQueue() {
        return commandQueue.back();
    }

   void GEVulkanCommandQueue::commitToGPU(){
       VkSubmitInfo submission {VK_STRUCTURE_TYPE_SUBMIT_INFO};
       submission.commandBufferCount = commandQueue.size();
       submission.pCommandBuffers = commandQueue.data();
       submission.pNext = nullptr;

       auto res = vkQueueSubmit(vkQueue, 1, &submission, submitFence);
       if(!VK_RESULT_SUCCEEDED(res)){
           printf("Failed to Submit Command Buffers to GPU");
           exit(1);
       };

       commandQueue.clear();

       vkWaitForFences(engine->device,1,&submitFence,VK_TRUE,UINT64_MAX);
   };

   GEVulkanCommandQueue::GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size):GECommandQueue(size){
       VkResult res;
       VkCommandPoolCreateInfo poolCreateInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};

       VkFenceCreateInfo fenceCreateInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
       fenceCreateInfo.pNext = nullptr;
       fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

       vkCreateFence(engine->device,&fenceCreateInfo,nullptr,&submitFence);

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