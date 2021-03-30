#include "GEVulkan.h"
#include "../GEPipeline.cpp"

#ifndef OMEGAGTE_VULKAN_GEVULKANPIPELINE_H
#define OMEGAGTE_VULKAN_GEVULKANPIPELINE_H

_NAMESPACE_BEGIN_

struct GEVulkanFunction : public __GEFunctionInternal {
    vk::UniqueShaderModule shaderModule;
    GEVulkanFunction(vk::UniqueShaderModule & shaderModule);
};

class GEVulkanRenderPipelineState : public __GERenderPipelineState {
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout layout;
public:
    GEVulkanRenderPipelineState(vk::UniquePipeline & pipeline,vk::UniquePipelineLayout & layout);
};

class GEVulkanComputePipelineState : public __GEComputePipelineState {
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout layout;
public:
    GEVulkanComputePipelineState(vk::UniquePipeline & pipeline,vk::UniquePipelineLayout & layout);
};

_NAMESPACE_END_

#endif