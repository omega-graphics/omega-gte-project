#include "GEVulkan.h"
#include "omegaGTE/GETexture.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANTEXTURE_H
#define OMEGAGTE_VULKAN_GEVULKANTEXTURE_H

_NAMESPACE_BEGIN_

class GEVulkanTexture : public GETexture {
public:
    vk::UniqueImage img;
    vk::UniqueImageView img_view;

    VmaAllocationInfo alloc_info;
    VmaAllocation alloc;


    GEVulkanTexture(vk::UniqueImage & img,vk::UniqueImageView & img_view,VmaAllocationInfo alloc_info,VmaAllocation alloc);
};

_NAMESPACE_END_

#endif