#include <vulkan/vulkan.hpp>
#include "omegaGE/GE.h"

namespace OmegaGE {
    #define VK_RESULT_SUCCEEDED(val) (val == vk::Result::eSuccess)
    class GEVulkanEngine : public OmegaGraphicsEngine {
    public:
        vk::Instance instance;
        vk::Device device;
        vk::PhysicalDevice physicalDevice;
        unsigned queueFamilyIndex;
        GEVulkanEngine();
        static SharedHandle<OmegaGraphicsEngine> Create();
    };

    
};