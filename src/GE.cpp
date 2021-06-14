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

GEColoredVertex::GEColoredVertex(FVector3D pos,FMatrix & color):GEVertex({pos}),color(color){

};

GEColoredVertex::GEColoredVertex(GEColoredVertex && vertex):GEVertex({vertex.pos}),color(vertex.color){
    
};

GEColoredVertex::GEColoredVertex(const GEColoredVertex & vertex):GEVertex({vertex.pos}),color(vertex.color){

};

GEColoredVertex::GEColoredVertex(GEColoredVertex & vertex):GEVertex({vertex.pos}),color(vertex.color){

};

GEColoredVertex GEColoredVertex::FromGPoint3D(GPoint3D &pt, FMatrix &color){
    std::cout << "Creating Point3D" << std::endl;
    return {{pt.x,pt.y,pt.z},color};
};

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
