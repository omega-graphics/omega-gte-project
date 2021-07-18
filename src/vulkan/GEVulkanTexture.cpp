#include "GEVulkanTexture.h"

_NAMESPACE_BEGIN_

GEVulkanTexture::GEVulkanTexture(
    vk::UniqueImage & img,
    vk::UniqueImageView & img_view,
    VmaAllocationInfo alloc_info,
    VmaAllocation alloc):
img(std::move(img)),
img_view(std::move(img_view)),
alloc_info(alloc_info),alloc(alloc)
{

};

_NAMESPACE_END_