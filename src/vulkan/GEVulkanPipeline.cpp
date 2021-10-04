#include "GEVulkanPipeline.h"

_NAMESPACE_BEGIN_

GTEVulkanShader::GTEVulkanShader(omegasl_shader & shader,VkShaderModule & module): GTEShader({shader}),shaderModule(module){};

GTEVulkanShader::~GTEVulkanShader(){
    vkDestroyShaderModule(parentEngine->device,shaderModule,nullptr);
};

GEVulkanRenderPipelineState::~GEVulkanRenderPipelineState() {
    vkDestroyPipeline(parentEngine->device,pipeline,nullptr);
    vkDestroyPipelineLayout(parentEngine->device,layout,nullptr);
    vkFreeDescriptorSets(parentEngine->device,descriptorPool,descs.size(),descs.data());
    for(auto & d : descLayouts) {
        vkDestroyDescriptorSetLayout(parentEngine->device,d,nullptr);
    }
    descs.clear();
    descMap.clear();
    vkDestroyDescriptorPool(parentEngine->device,descriptorPool,nullptr);
}

_NAMESPACE_END_