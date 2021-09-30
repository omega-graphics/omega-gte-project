#include "GEVulkan.h"
#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H
#define OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H


_NAMESPACE_BEGIN_
    class GEVulkanCommandQueue;

    class GEVulkanCommandBuffer : public GECommandBuffer {
        GEVulkanCommandQueue *parentQueue;
        VkCommandBuffer & commandBuffer;
        friend class GEVulkanCommandQueue;
    public:
        void startRenderPass(const GERenderPassDescriptor &desc) override;

        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState) override;

        void setScissorRects(std::vector<GEScissorRect> scissorRects) override;

        void setViewports(std::vector<GEViewport> viewports) override;

        void bindResourceAtVertexShader(SharedHandle<GEBuffer> &buffer, unsigned index) override;

        void bindResourceAtVertexShader(SharedHandle<GETexture> &texture, unsigned index) override;

        void bindResourceAtFragmentShader(SharedHandle<GEBuffer> &buffer, unsigned index) override;

        void bindResourceAtFragmentShader(SharedHandle<GETexture> &texture, unsigned  index) override;

        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned vertexCount, size_t startIdx) override;

        void finishRenderPass() override;

        void startComputePass(const GEComputePassDescriptor &desc) override;
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState) override;
        void finishComputePass() override;

        void startBlitPass() override;
        void finishBlitPass() override;
        void reset() override;
        GEVulkanCommandBuffer(VkCommandBuffer & commandBuffer,GEVulkanCommandQueue *parentQueue);
    };

    class GEVulkanCommandQueue : public GECommandQueue {
        GEVulkanEngine *engine;
        VkCommandPool commandPool;
        VkQueue commandQueue;
        OmegaCommon::Vector<VkCommandBuffer> commandBuffers;
        unsigned currentBufferIndex;
        friend class GEVulkanCommandBuffer;
    public:
        void submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer);
        void commitToGPU();
        void present();
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size);
    };
_NAMESPACE_END_

#endif