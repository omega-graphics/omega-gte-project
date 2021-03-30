#include "GEVulkanTexture.h"

_NAMESPACE_BEGIN_

GEVulkanTexture::GEVulkanTexture(vk::UniqueImage & img,vk::UniqueImageView & img_view):img(std::move(img)),img_view(std::move(img_view)){

};

_NAMESPACE_END_