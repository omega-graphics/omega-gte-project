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
    VkImageLayout layout;

    TextureDescriptor descriptor;

    VmaAllocationInfo alloc_info;
    VmaAllocation alloc;

    VmaMemoryUsage memoryUsage;

    void copyBytes(void *bytes, size_t len) override;


    GEVulkanTexture(GEVulkanEngine *engine,VkImage & img,VkImageView & img_view,VkImageLayout & layout,VmaAllocationInfo alloc_info,VmaAllocation alloc, const TextureDescriptor & descriptor,VmaMemoryUsage memoryUsage);
    ~GEVulkanTexture();
};

_NAMESPACE_END_

#endif