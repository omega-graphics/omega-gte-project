#include "GEVulkan.h"
#include "../GEPipeline.cpp"

#ifndef OMEGAGTE_VULKAN_GEVULKANPIPELINE_H
#define OMEGAGTE_VULKAN_GEVULKANPIPELINE_H

_NAMESPACE_BEGIN_

struct GTEVulkanShader : public GTEShader {
    VkShaderModule shaderModule;
    GTEVulkanShader(VkShaderModule & shaderModule);
};

class GEVulkanRenderPipelineState : public __GERenderPipelineState {
public:

    VkPipeline pipeline;
    VkPipelineLayout layout;

    OmegaCommon::Vector<VkDescriptorSetLayout> descLayouts;

    GEVulkanRenderPipelineState(VkPipeline pipeline,VkPipelineLayout & layout,OmegaCommon::Vector<VkDescriptorSetLayout> & descLayouts);
};

class GEVulkanComputePipelineState : public __GEComputePipelineState {
public:
    VkPipeline pipeline;
    VkPipelineLayout layout;
    
    GEVulkanComputePipelineState(VkPipeline & pipeline,VkPipelineLayout & layout,OmegaCommon::Vector<VkDescriptorSetLayout> & descLayouts);
};

_NAMESPACE_END_

#endif