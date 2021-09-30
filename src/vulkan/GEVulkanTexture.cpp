#include "GEVulkanTexture.h"
#include "vulkan/vulkan_core.h"

_NAMESPACE_BEGIN_

GEVulkanTexture::GEVulkanTexture(
    GEVulkanEngine *engine,
    VkImage & img,
    VkImageView & img_view,
    VmaAllocationInfo alloc_info,
    VmaAllocation alloc,VkDescriptorPool descPool):
engine(engine),
img(std::move(img)),
img_view(std::move(img_view)),
alloc_info(alloc_info),alloc(alloc),descPool(descPool)
{

};

void GEVulkanTexture::copyBytes(void *bytes, size_t len){
    void *ptr;
    vmaMapMemory(engine->memAllocator,alloc,&ptr);
    memcpy(ptr,bytes,len);
    vmaUnmapMemory(engine->memAllocator,alloc);
}

GEVulkanTexture::~GEVulkanTexture(){
    vmaDestroyImage(engine->memAllocator,img,alloc);
    vkDestroyImageView(engine->device,img_view,nullptr);
}

_NAMESPACE_END_