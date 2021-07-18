#include "GEVulkan.h"
#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H
#define OMEGAGTE_VULKAN_GEVULKANCOMMANDQUEUE_H


_NAMESPACE_BEGIN_
    class GEVulkanCommandQueue;

    class GEVulkanCommandBuffer : public GECommandBuffer {
        GEVulkanCommandQueue *parentQueue;
        vk::CommandBuffer & commandBuffer;
        friend class GEVulkanCommandQueue;
    public:
        void startRenderPass(const GERenderPassDescriptor &desc);

        void setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState);

        void setScissorRects(std::vector<GEScissorRect> scissorRects);

        void setViewports(std::vector<GEViewport> viewports);

        void setResourceConstAtVertexFunc(SharedHandle<GEBuffer> &buffer, unsigned index);

        void setResourceConstAtVertexFunc(SharedHandle<GETexture> &texture, unsigned index);

        void setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> &buffer, unsigned index);

        void setResourceConstAtFragmentFunc(SharedHandle<GETexture> &texture, unsigned  index);

        void drawPolygons(RenderPassDrawPolygonType polygonType, unsigned vertexCount, size_t startIdx);

        void finishRenderPass();

        void startComputePass(const GEComputePassDescriptor &desc);
        void setComputePipelineState(SharedHandle<GEComputePipelineState> &pipelineState);
        void finishComputePass();

        void startBlitPass();
        void finishBlitPass();
        void reset();
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
        void submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer);
        void commitToGPU();
        void present();
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEVulkanCommandQueue(GEVulkanEngine *engine,unsigned size);
    };
_NAMESPACE_END_

#endif