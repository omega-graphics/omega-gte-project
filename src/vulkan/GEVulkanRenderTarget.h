#include "GEVulkan.h"
#include "omegaGTE/GERenderTarget.h"

#include "GEVulkanTexture.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANRENDERTARGET_H
#define OMEGAGTE_VULKAN_GEVULKANRENDERTARGET_H

_NAMESPACE_BEGIN_

class GEVulkanNativeRenderTarget : public GENativeRenderTarget {
public:
    Window x11_window;
    VkSurfaceKHR surface;
    VkFramebuffer frameBuffer;
    VkSwapchainKHR swapchainKHR;
    
};

class GEVulkanTextureRenderTarget : public GETextureRenderTarget {
public:
    SharedHandle<GEVulkanTexture> texture;
    VkFramebuffer frameBuffer;
};

_NAMESPACE_END_

#endif