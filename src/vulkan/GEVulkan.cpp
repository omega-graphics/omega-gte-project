#include "GEVulkan.h"
#include "GEVulkanCommandQueue.h"
#include "GEVulkanTexture.h"
#include "GEVulkanPipeline.h"
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <memory>

#include "OmegaGTE.h"
#include "vulkan/vulkan_core.h"


_NAMESPACE_BEGIN_

    static VkInstance instance;
    bool vulkanInit = false;

    void initVulkan(){
        vkCreateInstance(nullptr,nullptr,&instance);
        vulkanInit = true;
    }

    void cleanupVulkan(){
        vkDestroyInstance(instance,nullptr);
        vulkanInit = false;
    }

    struct GTEVulkanDevice : public GTEDevice {
        VkPhysicalDevice device;
        GTEVulkanDevice(GTEDevice::Type type,const char *name,GTEDeviceFeatures & features,VkPhysicalDevice &device) : GTEDevice(type,name,features),device(device) {

        };
        ~GTEVulkanDevice() = default;
    };

    OmegaCommon::Vector<SharedHandle<GTEDevice>> enumerateDevices(){
        OmegaCommon::Vector<SharedHandle<GTEDevice>> devs;
        if(!vulkanInit){
            initVulkan();
        }
        OmegaCommon::Vector<VkPhysicalDevice> vk_devs;
        std::uint32_t device_count;
        vkEnumeratePhysicalDevices(instance,&device_count,nullptr);
        vk_devs.resize(device_count);
        vkEnumeratePhysicalDevices(instance,&device_count,vk_devs.data());
        for(auto dev : vk_devs){
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(dev,&props);
            GTEDeviceFeatures features {false};
            GTEDevice::Type type = GTEDevice::Discrete;
            if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
                type = GTEDevice::Discrete;
            }
            else if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
                type = GTEDevice::Integrated;
            }
            devs.emplace_back(SharedHandle<GTEDevice>(new GTEVulkanDevice(type,props.deviceName,features,dev)));
        }
        return devs;
    }


    SharedHandle<GTEShader> GEVulkanEngine::_loadShaderFromDesc(omegasl_shader *shaderDesc){
        VkShaderModuleCreateInfo shaderModuleDesc {};

        shaderModuleDesc.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleDesc.pNext = nullptr;
        shaderModuleDesc.pCode = (std::uint32_t *)shaderDesc->data;
        shaderModuleDesc.codeSize = shaderDesc->dataSize;
        shaderModuleDesc.flags = 0;

        VkShaderModule module;
        vkCreateShaderModule(device,&shaderModuleDesc,nullptr,&module);
        return SharedHandle<GTEShader>(new GTEVulkanShader(module));
    }


    GEVulkanEngine::GEVulkanEngine(SharedHandle<GTEVulkanDevice> device){
        VkResult res;
        OmegaCommon::Vector<VkExtensionProperties> ext_props;
        std::uint32_t count;
        vkEnumerateInstanceExtensionProperties(nullptr,&count,nullptr);
        ext_props.resize(count);
        vkEnumerateInstanceExtensionProperties(nullptr,&count,ext_props.data());

        OmegaCommon::Vector<VkLayerProperties> layer_props;
        count = 0;
        vkEnumerateInstanceLayerProperties(&count,nullptr);
        layer_props.resize(count);
        vkEnumerateInstanceLayerProperties(&count,layer_props.data());

        count = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&count,nullptr);
        queueFamilyProps.resize(count);

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&count,queueFamilyProps.data());

        OmegaCommon::Vector<VkDeviceQueueCreateInfo> deviceQueues;
        unsigned id = 0;
        for(auto & q : queueFamilyProps){
            if(q.queueFlags & VK_QUEUE_GRAPHICS_BIT || q.queueFlags & VK_QUEUE_COMPUTE_BIT){
                queueFamilyIndicies.push_back(id);
            }
            VkDeviceQueueCreateInfo queueInfo {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            queueInfo.pNext = nullptr;
            queueInfo.queueFamilyIndex = id;
            queueInfo.queueCount = q.queueCount;
            ++id;
        }

        

        VkDeviceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pNext = nullptr;
        info.pQueueCreateInfos = deviceQueues.data();
        info.queueCreateInfoCount = deviceQueues.size();
        info.enabledExtensionCount = ext_props.size();
        info.enabledLayerCount = layer_props.size();

        vkCreateDevice(physicalDevice,&info,nullptr,&this->device);


        VmaAllocatorCreateInfo allocator_info {};
        allocator_info.instance = instance;
        allocator_info.device = this->device;
        allocator_info.physicalDevice = physicalDevice;
        allocator_info.vulkanApiVersion = VK_API_VERSION_1_2;
        auto _res = vmaCreateAllocator(&allocator_info,&memAllocator);

        if(_res != VK_SUCCESS){
            printf("Failed to Create Allocator");
            exit(1);
        };
      

        resource_count = 0;
       
        DEBUG_STREAM("Successfully Created GEVulkanEngine");
    };

    SharedHandle<OmegaGraphicsEngine> GEVulkanEngine::Create(SharedHandle<GTEDevice> & device){
        return SharedHandle<OmegaGraphicsEngine>(new GEVulkanEngine(std::dynamic_pointer_cast<GTEVulkanDevice>(device)));
    };

    SharedHandle<GECommandQueue> GEVulkanEngine::makeCommandQueue(unsigned int maxBufferCount){
        return std::make_shared<GEVulkanCommandQueue>(this,maxBufferCount);
    };

    SharedHandle<GEBuffer> GEVulkanEngine::makeBuffer(const BufferDescriptor &desc){
        VkBufferCreateInfo buffer_desc;
       
        buffer_desc.flags = 0;
        buffer_desc.size = desc.len;
        buffer_desc.sharingMode = VK_SHARING_MODE_CONCURRENT;

        VmaAllocationCreateInfo alloc_info {};
        switch (desc.usage) {
            case BufferDescriptor::Upload : {
                alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                break;
            }
            case BufferDescriptor::Readback : {
                alloc_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                break;
            }
        }

        alloc_info.priority = 0;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;

        VkBuffer buffer;
        
        vmaCreateBuffer(memAllocator,&buffer_desc,&alloc_info,&buffer,&allocation,&allocationInfo);

        VkBufferViewCreateInfo bufferViewInfo {};
        bufferViewInfo.format = VK_FORMAT_UNDEFINED;
        bufferViewInfo.buffer = buffer;
        bufferViewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        bufferViewInfo.offset = 0;
        bufferViewInfo.range = VK_WHOLE_SIZE;

        VkBufferView bufferView;

        vkCreateBufferView(device,&bufferViewInfo,nullptr,&bufferView);

        VkDescriptorPoolCreateInfo poolCreate {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};

        poolCreate.poolSizeCount = 1;
        VkDescriptorPoolSize poolSize {};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        poolSize.descriptorCount = 1;

        poolCreate.pPoolSizes = &poolSize;
        poolCreate.maxSets = 1;
        poolCreate.pNext = nullptr;

        VkDescriptorPool descPool;

        vkCreateDescriptorPool(device,&poolCreate,nullptr,&descPool);

               
       
        return SharedHandle<GEBuffer>(new GEVulkanBuffer(this,buffer,bufferView,allocation,allocationInfo,descPool));
    };
    SharedHandle<GEHeap> GEVulkanEngine::makeHeap(const HeapDescriptor &desc){
        return nullptr;
    };

    SharedHandle<GETexture>GEVulkanEngine::makeTexture(const TextureDescriptor &desc){
        VkImageCreateInfo image_desc;
        image_desc.queueFamilyIndexCount = queueFamilyIndicies.size();
        image_desc.pQueueFamilyIndices = queueFamilyIndicies.data();

        VkFormat image_format;

        switch (desc.pixelFormat) {
            case TexturePixelFormat::RGBA8Unorm : {
                image_format = VK_FORMAT_R8G8B8A8_UNORM;
                break;
            }
            case TexturePixelFormat::RGBA16Unorm : {
                image_format = VK_FORMAT_R16G16B16A16_UNORM;
                break;
            }
            case TexturePixelFormat::RGBA8Unorm_SRGB : {
                image_format = VK_FORMAT_R8G8B8A8_SRGB;
                break;
            }
        }

        image_desc.format = image_format;

        VkImageType type;
        VkImageViewType viewType;
        
        switch (desc.type) {
            case GETexture::Texture1D : {
                type = VK_IMAGE_TYPE_1D;
                viewType = VK_IMAGE_VIEW_TYPE_1D;
                break;
            }
            case GETexture::Texture2D : {
                type = VK_IMAGE_TYPE_2D;
                viewType = VK_IMAGE_VIEW_TYPE_2D;
                break;
            }
            case GETexture::Texture3D : {
                type = VK_IMAGE_TYPE_3D;
                viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;
            }
        }

        image_desc.imageType = type;
        image_desc.extent.width = desc.width;
        image_desc.extent.height = desc.height;
        image_desc.extent.depth = desc.depth;
        image_desc.mipLevels = desc.mipLevels;
        image_desc.arrayLayers = 1;

        VkImageUsageFlags usageFlags;
        VkImageLayout layout;
        VmaMemoryUsage memoryUsage;

        VkDescriptorType descType;

        switch (desc.usage) {
            case GETexture::GPURead : {
                usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                descType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                break;
            }
            case GETexture::GPUWrite : {
                usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            }
            case GETexture::RenderTarget : {
                usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
                layout = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;
                descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            }
            case GETexture::MSResolveDest : {
                usageFlags = VK_IMAGE_USAGE_STORAGE_BIT;
                memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
                layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            }
        }

        image_desc.usage = usageFlags;
        image_desc.initialLayout = layout;

        image_desc.sharingMode = VK_SHARING_MODE_CONCURRENT;
        
    
        VkImage image;
        VkImageView imageView;
        

        VmaAllocationCreateInfo create_alloc_info;
        create_alloc_info.usage = memoryUsage;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;

        vmaCreateImage(memAllocator,&image_desc,&create_alloc_info,&image,&alloc,&alloc_info);
        

        VkImageViewCreateInfo image_view_desc;

        image_view_desc.viewType = viewType;
        image_view_desc.image = image;
        image_view_desc.flags = 0;
        image_view_desc.format = image_format;

        vkCreateImageView(device,&image_view_desc,nullptr,&imageView);

        VkDescriptorPoolCreateInfo poolCreate {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};

        poolCreate.poolSizeCount = 1;
        VkDescriptorPoolSize poolSize {};
        poolSize.type = descType;
        poolSize.descriptorCount = 1;

        poolCreate.pPoolSizes = &poolSize;
        poolCreate.maxSets = 1;
        poolCreate.pNext = nullptr;

        VkDescriptorPool descPool;

        vkCreateDescriptorPool(device,&poolCreate,nullptr,&descPool);
        

        return SharedHandle<GETexture>(new GEVulkanTexture(this,image,imageView,alloc_info,alloc,descPool));
    };

    VkPipelineLayout GEVulkanEngine::createPipelineLayoutFromShaderDescs(unsigned shaderN,const omegasl_shader *shaders,OmegaCommon::Vector<VkDescriptorSetLayout> & descLayout){
        VkPipelineLayoutCreateInfo layout_info {};

        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.pNext = nullptr;

        VkDescriptorSetLayoutCreateInfo desc_layout_info{};
        VkDescriptorSetLayoutBinding b;
        b.descriptorCount = 1;

        OmegaCommon::ArrayRef<omegasl_shader> shadersArr {shaders,shaders + shaderN};

        OmegaCommon::Vector<VkDescriptorSetLayoutBinding> bindings;

        for(auto & s : shadersArr){
            VkShaderStageFlags shaderStageFlags;

            if(s.type == OMEGASL_SHADER_VERTEX){
                shaderStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            }
            else if(s.type == OMEGASL_SHADER_FRAGMENT){
                shaderStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            else {
                shaderStageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            }

            OmegaCommon::ArrayRef<omegasl_shader_layout_desc> layouts {s.pLayout,s.pLayout + s.nLayout};
            VkDescriptorSetLayout set_layout;
            for(auto & l : layouts){
                b.pImmutableSamplers = nullptr;

                switch (l.type) {
                    case OMEGASL_SHADER_BUFFER_DESC : {
                          b.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                       
                        break;
                    }
                    case OMEGASL_SHADER_SAMPLER1D_DESC : 
                    case OMEGASL_SHADER_SAMPLER2D_DESC :
                    case OMEGASL_SHADER_SAMPLER3D_DESC : {
                        b.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                        break;
                    }
                    case OMEGASL_SHADER_TEXTURE1D_DESC :
                    case OMEGASL_SHADER_TEXTURE2D_DESC :
                    case OMEGASL_SHADER_TEXTURE3D_DESC : {
                        if(l.io_mode == OMEGASL_SHADER_DESC_IO_IN){
                            b.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                        }
                        else {
                            b.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                        }
                        break;
                    }
                }
                b.binding = l.gpu_relative_loc;
                b.stageFlags = shaderStageFlags;
                bindings.push_back(b);
            }
            desc_layout_info.pNext = nullptr;
            desc_layout_info.bindingCount = bindings.size();
            desc_layout_info.pBindings = bindings.data();
            desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            vkCreateDescriptorSetLayout(device,&desc_layout_info,nullptr,&set_layout);
            descLayout.push_back(set_layout);
            bindings.clear();
        }

        layout_info.pSetLayouts = descLayout.data();
        layout_info.setLayoutCount = descLayout.size();
        layout_info.pushConstantRangeCount = 0;
        layout_info.pPushConstantRanges = nullptr;

        VkPipelineLayout pipeline_layout;

        vkCreatePipelineLayout(device,&layout_info,nullptr,&pipeline_layout);

        return pipeline_layout;

    }


    SharedHandle<GERenderPipelineState> GEVulkanEngine::makeRenderPipelineState(RenderPipelineDescriptor &desc){
        
        omegasl_shader shaders[] = {desc.vertexFunc->internal,desc.fragmentFunc->internal};
        
        OmegaCommon::Vector<VkDescriptorSetLayout> descLayout;

        VkPipelineLayout layout = createPipelineLayoutFromShaderDescs(2,shaders,descLayout);        

        VkGraphicsPipelineCreateInfo createInfo {};
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;
        createInfo.layout = layout;
        createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        VkPipelineRasterizationStateCreateInfo rasterState {};
        rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        switch(desc.cullMode){
            case RasterCullMode::None : {
                rasterState.cullMode = VK_CULL_MODE_NONE;
                break;
            }
            case RasterCullMode::Front : {
                rasterState.cullMode = VK_CULL_MODE_FRONT_BIT;
                break;
            }
            case RasterCullMode::Back : {
                rasterState.cullMode = VK_CULL_MODE_BACK_BIT;
                break;
            }
        }


        VkPipelineDynamicStateCreateInfo dynamicState {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;


        auto *vertexShader = (GTEVulkanShader *)desc.vertexFunc.get();
        VkPipelineShaderStageCreateInfo vertexStage {};
        vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexStage.module = vertexShader->shaderModule;
        vertexStage.pName = "main";

        auto *fragmentShader = (GTEVulkanShader *)desc.fragmentFunc.get();
        VkPipelineShaderStageCreateInfo fragmentStage {};
        fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentStage.module = fragmentShader->shaderModule;
        fragmentStage.pName = "main";
        
        VkPipelineShaderStageCreateInfo stages[] = {vertexStage,fragmentStage};
        
        createInfo.pStages = stages;
        createInfo.stageCount = 2;
        createInfo.pDynamicState = &dynamicState;
        createInfo.pRasterizationState = &rasterState;

        VkPipeline pipeline;

        vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&createInfo,nullptr,&pipeline);
      
        return SharedHandle<GERenderPipelineState>(new GEVulkanRenderPipelineState(pipeline,layout,descLayout));
    };
    SharedHandle<GEComputePipelineState> GEVulkanEngine::makeComputePipelineState(ComputePipelineDescriptor &desc){
        OmegaCommon::Vector<VkDescriptorSetLayout> descLayouts;
        auto pipeline_layout = createPipelineLayoutFromShaderDescs(1,&desc.computeFunc->internal,descLayouts);

        

        VkComputePipelineCreateInfo pipeline_desc {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipeline_desc.basePipelineIndex = -1;
        pipeline_desc.basePipelineHandle = VK_NULL_HANDLE;
        auto *computeShader = (GTEVulkanShader *)desc.computeFunc.get();
        VkPipelineShaderStageCreateInfo computeStage {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeStage.module = computeShader->shaderModule;
        computeStage.pName = "main";
        
        pipeline_desc.stage = computeStage;
        pipeline_desc.layout = pipeline_layout;
         

         VkPipeline pipeline;
         auto result = vkCreateComputePipelines(device,VK_NULL_HANDLE,1,&pipeline_desc,nullptr,&pipeline);
         if(!VK_RESULT_SUCCEEDED(result)){
            exit(1);
        };
        return SharedHandle<GEComputePipelineState>(new GEVulkanComputePipelineState(pipeline,pipeline_layout,descLayouts));
    };

    SharedHandle<GEFence> GEVulkanEngine::makeFence(){
        
        return nullptr;
    };

    SharedHandle<GENativeRenderTarget> GEVulkanEngine::makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc){
        return nullptr;
    };

    SharedHandle<GETextureRenderTarget> GEVulkanEngine::makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc){
        return nullptr;
    };

    GEVulkanEngine::~GEVulkanEngine(){
        vkDestroyDevice(device,nullptr);
        vmaDestroyAllocator(memAllocator);
    }



_NAMESPACE_END_