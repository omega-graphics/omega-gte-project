#include "GEVulkan.h"
#include "omegaGTE/GETexture.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANTEXTURE_H
#define OMEGAGTE_VULKAN_GEVULKANTEXTURE_H

_NAMESPACE_BEGIN_

class GEVulkanTexture : public GETexture {
    vk::UniqueImage img;
    vk::UniqueImageView img_view;
    public:
    GEVulkanTexture(vk::UniqueImage & img,vk::UniqueImageView & img_view);
};

_NAMESPACE_END_

#endif