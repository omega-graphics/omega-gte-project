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
public:
    vk::UniquePipelineCache cache;

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    vk::UniquePipelineLayout layout;

    GEVulkanRenderPipelineState(vk::GraphicsPipelineCreateInfo & pipelineInfo,vk::UniquePipelineLayout & layout,vk::UniquePipelineCache & cache);
};

class GEVulkanComputePipelineState : public __GEComputePipelineState {
public:
    vk::GraphicsPipelineCreateInfo pipeline;
    vk::UniquePipelineLayout layout;
    
    GEVulkanComputePipelineState(vk::UniquePipeline & pipeline,vk::UniquePipelineLayout & layout);
};

_NAMESPACE_END_

#endif