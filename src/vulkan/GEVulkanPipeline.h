#include "GEVulkan.h"
#include "../GEPipeline.cpp"

#ifndef OMEGAGTE_VULKAN_GEVULKANPIPELINE_H
#define OMEGAGTE_VULKAN_GEVULKANPIPELINE_H

_NAMESPACE_BEGIN_

struct GTEVulkanShader : public GTEShader {
    GEVulkanEngine *parentEngine;
    VkShaderModule shaderModule;
    GTEVulkanShader(omegasl_shader & shader,VkShaderModule & shaderModule);
    ~GTEVulkanShader();
};

class GEVulkanRenderPipelineState : public __GERenderPipelineState {
    GEVulkanEngine *parentEngine;
public:

    VkPipeline pipeline;
    VkPipelineLayout layout;

    VkDescriptorPool descriptorPool;

    OmegaCommon::Vector<VkDescriptorSetLayout> descLayouts;

    OmegaCommon::Map<unsigned,VkDescriptorSet> descMap;

    OmegaCommon::Vector<VkDescriptorSet> descs;

    GEVulkanRenderPipelineState(VkPipeline & pipeline,VkPipelineLayout & layout,OmegaCommon::Map<unsigned,VkDescriptorSet> & descMap,OmegaCommon::Vector<VkDescriptorSet> descs);
    ~GEVulkanRenderPipelineState();
};

class GEVulkanComputePipelineState : public __GEComputePipelineState {
    GEVulkanEngine *parentEngine;
public:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    VkDescriptorPool descriptorPool;

    OmegaCommon::Vector<VkDescriptorSetLayout> descLayouts;

    OmegaCommon::Map<unsigned,VkDescriptorSet> descMap;

    OmegaCommon::Vector<VkDescriptorSet> descs;
    
    GEVulkanComputePipelineState(VkPipeline & pipeline,VkPipelineLayout & layout,OmegaCommon::Map<unsigned,VkDescriptorSet> & descMap,OmegaCommon::Vector<VkDescriptorSet> descs);
    ~GEVulkanComputePipelineState();
};

_NAMESPACE_END_

#endif