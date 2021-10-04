#include "GEVulkanTexture.h"
#include "vulkan/vulkan_core.h"

_NAMESPACE_BEGIN_

GEVulkanTexture::GEVulkanTexture(
    GEVulkanEngine *engine,
    VkImage & img,
    VkImageView & img_view,
    VkImageLayout & layout,
    VmaAllocationInfo alloc_info,
    VmaAllocation alloc,const TextureDescriptor & descriptor,VmaMemoryUsage memoryUsage):
engine(engine),
img(std::move(img)),
img_view(std::move(img_view)),
layout(layout),
descriptor(descriptor),
alloc_info(alloc_info),alloc(alloc),memoryUsage(memoryUsage)
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