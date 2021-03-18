#include "GEVulkan.h"

namespace OmegaGE {
    GEVulkanEngine::GEVulkanEngine(){
        vk::Result res;
        auto ext_props = vk::enumerateInstanceExtensionProperties();
        auto layer_props = vk::enumerateInstanceLayerProperties();
        vk::InstanceCreateInfo createInstInfo;
       res = vk::createInstance(&createInstInfo,nullptr,&instance);
       if(!VK_RESULT_SUCCEEDED(res)){
          exit(1);
       }
      
       
       auto physicalDevices = instance.enumeratePhysicalDevices();
       physicalDevice = physicalDevices[0];
       vk::DeviceCreateInfo createDeviceInfo;
       res = physicalDevice.createDevice(&createDeviceInfo,nullptr,&device);
       if(!VK_RESULT_SUCCEEDED(res)){
          exit(1);
       }
       

     
    };
    SharedHandle<OmegaGraphicsEngine> GEVulkanEngine::Create(){
        return std::make_shared<GEVulkanEngine>();
    };
};