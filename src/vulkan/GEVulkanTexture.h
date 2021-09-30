#include "GEVulkan.h"
#include "omegaGTE/GETexture.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANTEXTURE_H
#define OMEGAGTE_VULKAN_GEVULKANTEXTURE_H

_NAMESPACE_BEGIN_

class GEVulkanTexture : public GETexture {
    GEVulkanEngine *engine;
public:
    VkImage img;
    VkImageView img_view;

    VmaAllocationInfo alloc_info;
    VmaAllocation alloc;

    VkDescriptorPool descPool;

    void copyBytes(void *bytes, size_t len) override;


    GEVulkanTexture(GEVulkanEngine *engine,VkImage & img,VkImageView & img_view,VmaAllocationInfo alloc_info,VmaAllocation alloc,VkDescriptorPool descPool);
    ~GEVulkanTexture();
};

_NAMESPACE_END_

#endif