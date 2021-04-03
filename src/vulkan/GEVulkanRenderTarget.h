#include "GEVulkan.h"
#include "omegaGTE/GERenderTarget.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANRENDERTARGET_H
#define OMEGAGTE_VULKAN_GEVULKANRENDERTARGET_H

_NAMESPACE_BEGIN_

class GEVulkanNativeRenderTarget : public GENativeRenderTarget {
    Window x11_window;
    vk::UniqueSurfaceKHR surfaceKHR;
    vk::UniqueFramebuffer frameBuffer;
    vk::UniqueSwapchainKHR swapchainKHR;
public:
    
};

_NAMESPACE_END_

#endif