#include "omegaGTE/GE.h"

#ifdef TARGET_DIRECTX
#include "d3d12/GED3D12.h"
#endif

#ifdef TARGET_METAL
#include "metal/GEMetal.h"
#endif

#ifdef TARGET_VULKAN
#include "vulkan/GEVulkan.h"
#endif

_NAMESPACE_BEGIN_

SharedHandle<OmegaGraphicsEngine> OmegaGraphicsEngine::Create(){
    #ifdef TARGET_METAL
        return CreateMetalEngine();
    #endif
    #ifdef TARGET_DIRECTX
        return GED3D12Engine::Create();
    #endif
    #ifdef TARGET_VULKAN
        return GEVulkanEngine::Create();
    #endif
};

_NAMESPACE_END_
