#include "GEVulkan.h"
#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H
#define OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H


_NAMESPACE_BEGIN_
    class GEVulkanCommandQueue;
    class GEVulkanRenderPipelineState;
    class GEVulkanComputePipelineState;
    class GEVulkanTexture;
    class GEVulkanBuffer;

    class GEVulkanCommandBuffer : public GECommandBuffer {
        GEVulkanCommandQueue *parentQueue;
        VkCommandBuffer & commandBuffer;

        GEVulkanRenderPipelineState *renderPipelineState = nullptr;
        GEVulkanComputePipelineState *computePipelineState = nullptr;

        friend class GEVulkanCommandQueue;

        bool inBlitPass = false;
        bool inComputePass = false;

        unsigned getBindingForResourceID(unsigned & id,omegasl_shader & shader);
        unsigned getDescriptorSetIndexForResourceID(unsigned & id);

        omegasl_shader_layout_desc_io_mode getResourceIOModeForResourceID(unsigned & id,omegasl_shader & shader);

        void insertResourceBarrierIfNeeded(GEVulkanTexture *texture,unsigned & resource_id,omegasl_shader & shader);
        void insertResourceBarrierIfNeeded(GEVulkanBuffer *buffer,unsigned & resource_id,omegasl_shader & shader);
    public:
        void waitForFence(SharedHandle<GEFence> &fence, unsigned int val) override;
        void signalFence(SharedHandle<GEFence> &fence, unsigned int val) override;

        void startRenderPass(const GERenderPassDescriptor &desc) override;

        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState) override;

        void setScissorRects(std::vector<GEScissorRect> scissorRects) override;

        void setViewports(std::vector<GEViewport> viewports) override;

        void bindResourceAtVertexShader(SharedHandle<GEBuffer> &buffer, unsigned index) override;

        void bindResourceAtVertexShader(SharedHandle<GETexture> &texture, unsigned index) override;

        void bindResourceAtFragmentShader(SharedHandle<GEBuffer> &buffer, unsigned index) override;

        void bindResourceAtFragmentShader(SharedHandle<GETexture> &texture, unsigned  index) override;

        void setVertexBuffer(SharedHandle<GEBuffer> &buffer) override;

        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned vertexCount, size_t startIdx) override;

        void finishRenderPass() override;

        void startComputePass(const GEComputePassDescriptor &desc) override;
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState) override;
        void bindResourceAtComputeShader(SharedHandle<GEBuffer> &buffer, unsigned int id) override;
        void bindResourceAtComputeShader(SharedHandle<GETexture> &texture, unsigned int id) override;
        void dispatchThreads(unsigned int x, unsigned int y, unsigned int z) override;
        void finishComputePass() override;

        void startBlitPass() override;
        void copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest) override;
        void copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest, const TextureRegion &region, const GPoint3D &destCoord) override;
        void finishBlitPass() override;
        void reset() override;
        GEVulkanCommandBuffer(VkCommandBuffer & commandBuffer,GEVulkanCommandQueue *parentQueue);
        ~GEVulkanCommandBuffer() override = default;
    };

    class GEVulkanCommandQueue : public GECommandQueue {
        GEVulkanEngine *engine;
        VkCommandPool commandPool;

        VkFence submitFence;

        OmegaCommon::Vector<VkCommandBuffer> commandBuffers;

        OmegaCommon::Vector<VkCommandBuffer> commandQueue;
        unsigned currentBufferIndex;
        friend class GEVulkanCommandBuffer;
    public:
        VkQueue vkQueue;
        void submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer);
        void commitToGPU();
        void present();
        VkCommandBuffer &getLastCommandBufferInQueue();
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size);
        ~GEVulkanCommandQueue();
    };
_NAMESPACE_END_

#endif