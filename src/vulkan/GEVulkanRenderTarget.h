#include "GEVulkan.h"
#include "omegaGTE/GERenderTarget.h"

#include "GEVulkanTexture.h"

#ifndef OMEGAGTE_VULKAN_GEVULKANRENDERTARGET_H
#define OMEGAGTE_VULKAN_GEVULKANRENDERTARGET_H

_NAMESPACE_BEGIN_

class GEVulkanNativeRenderTarget : public GENativeRenderTarget {
    GEVulkanEngine *parentEngine;
public:
#ifdef VK_USE_PLATFORM_XLIB_KHR
    Window x11_window;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    wl_surface * wlSurface;
#endif
    VkSurfaceKHR surface;
    VkFramebuffer frameBuffer;
    VkSwapchainKHR swapchainKHR;
};

class GEVulkanTextureRenderTarget : public GETextureRenderTarget {
    GEVulkanEngine *parentEngine;
public:
    SharedHandle<GEVulkanTexture> texture;
    VkFramebuffer frameBuffer;
};

_NAMESPACE_END_

#endif