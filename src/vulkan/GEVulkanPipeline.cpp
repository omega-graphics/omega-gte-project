#include "GEVulkanPipeline.h"

_NAMESPACE_BEGIN_

GEVulkanRenderPipelineState::GEVulkanRenderPipelineState(
    vk::GraphicsPipelineCreateInfo & pipelineInfo,
    vk::UniquePipelineLayout & layout,
    vk::UniquePipelineCache & cache):
    pipelineInfo(pipelineInfo),
cache(std::move(cache)),
layout(std::move(layout)){};

// GEVulkanComputePipelineState::GEVulkanComputePipelineState(vk::UniquePipeline & pipeline,vk::UniquePipelineLayout & layout):pipeline(std::move(pipeline)),layout(std::move(layout)){};

GEVulkanFunction::GEVulkanFunction(vk::UniqueShaderModule & shaderModule):shaderModule(std::move(shaderModule)){};

_NAMESPACE_END_