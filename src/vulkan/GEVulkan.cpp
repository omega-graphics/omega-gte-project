#include "GEVulkan.h"
#include "GEVulkanCommandQueue.h"
#include "GEVulkanTexture.h"
#include "GEVulkanPipeline.h"
#include <initializer_list>
#include <iostream>
_NAMESPACE_BEGIN_


    GEVulkanEngine::GEVulkanEngine(){
        vk::Result res;
        auto ext_props = vk::enumerateInstanceExtensionProperties();
        auto layer_props = vk::enumerateInstanceLayerProperties();
        vk::InstanceCreateInfo createInstInfo;
       res = vk::createInstance(&createInstInfo,nullptr,&instance);
       if(!VK_RESULT_SUCCEEDED(res)){
          exit(1);
       }

        VmaAllocatorCreateInfo info;
        info.instance = instance;
        info.device = device;
        info.physicalDevice = physicalDevice;
        info.vulkanApiVersion = VK_API_VERSION_1_2;
        auto _res = vmaCreateAllocator(&info,&memAllocator);

        if(_res != VK_SUCCESS){
            printf("Failed to Create Allocator");
            exit(1);
        };
      
       
       auto physicalDevices = instance.enumeratePhysicalDevices();
       physicalDevice = physicalDevices[0];
       vk::DeviceCreateInfo createDeviceInfo;
       res = physicalDevice.createDevice(&createDeviceInfo,nullptr,&device);
       if(!VK_RESULT_SUCCEEDED(res)){
          exit(1);
       }
        DEBUG_STREAM("Successfully Created GEVulkanEngine");
    };

    SharedHandle<OmegaGraphicsEngine> GEVulkanEngine::Create(){
        return std::make_shared<GEVulkanEngine>();
    };

    SharedHandle<GECommandQueue> GEVulkanEngine::makeCommandQueue(unsigned int maxBufferCount){
        return std::make_shared<GEVulkanCommandQueue>(this,maxBufferCount);
    };

    SharedHandle<GEBuffer> GEVulkanEngine::makeBuffer(const BufferDescriptor &desc){
        vk::BufferCreateInfo buffer_desc;
        using vk::BufferUsageFlagBits;
        buffer_desc.flags = {};
        buffer_desc.size = desc.len;
        buffer_desc.sharingMode = vk::SharingMode::eExclusive;
        buffer_desc.usage = BufferUsageFlagBits::eStorageBuffer | BufferUsageFlagBits::eVertexBuffer;
        vk::UniqueBuffer buffer = device.createBufferUnique(buffer_desc);
        return std::make_shared<GEVulkanBuffer>(buffer,memAllocator);
    };
    SharedHandle<GEHeap> GEVulkanEngine::makeHeap(const HeapDescriptor &desc){
        return nullptr;
    };

    SharedHandle<GETexture>GEVulkanEngine::makeTexture(const TextureDescriptor &desc){
        vk::ImageCreateInfo image_desc;
        image_desc.queueFamilyIndexCount = 1;
        image_desc.pQueueFamilyIndices = &queueFamilyIndex;
        image_desc.format = vk::Format::eR8G8B8A8Unorm;
        image_desc.imageType = (desc.type == GETexture::Texture2D)? vk::ImageType::e2D : vk::ImageType::e3D;
        image_desc.extent.width = desc.width;
        image_desc.extent.height = desc.height;
        image_desc.extent.depth = desc.depth;
        image_desc.mipLevels = 1;
        image_desc.sharingMode = vk::SharingMode::eExclusive;
        using vk::ImageUsageFlagBits;
        image_desc.usage = ImageUsageFlagBits::eTransferDst | ImageUsageFlagBits::eTransferSrc;
        image_desc.initialLayout = vk::ImageLayout::eGeneral;
        vk::UniqueImage img = device.createImageUnique(image_desc);

        VmaAllocationCreateInfo create_alloc_info;
        create_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;
        vmaAllocateMemoryForImage(memAllocator,img.get(),&create_alloc_info,&alloc,&alloc_info);

        vk::ImageViewCreateInfo image_view_desc;
        image_view_desc.viewType = vk::ImageViewType::e2D;
        image_view_desc.image = img.get();
        using vk::ImageViewCreateFlagBits;
        image_view_desc.flags = ImageViewCreateFlagBits::eFragmentDensityMapDynamicEXT;
        image_view_desc.format = vk::Format::eR8G8B8A8Unorm;
        vk::UniqueImageView img_view = device.createImageViewUnique(image_view_desc);

        return std::make_shared<GEVulkanTexture>(img,img_view,alloc_info,alloc);
    };

    SharedHandle<GERenderPipelineState> GEVulkanEngine::makeRenderPipelineState(RenderPipelineDescriptor &desc){
        vk::PipelineLayoutCreateInfo layout_info;
        layout_info.pSetLayouts = nullptr;
        layout_info.setLayoutCount = 0;
        layout_info.pushConstantRangeCount = 0;
        layout_info.pPushConstantRanges = nullptr;
    
        vk::UniquePipelineLayout pipeline_layout = device.createPipelineLayoutUnique(layout_info);

        vk::GraphicsPipelineCreateInfo pipeline_desc;
        pipeline_desc.basePipelineIndex = -1;

        GEVulkanFunction *vertexFunc = (GEVulkanFunction *)desc.vertexFunc;
        vk::PipelineShaderStageCreateInfo vertexStage;
        vertexStage.stage = vk::ShaderStageFlagBits::eVertex;
        vertexStage.module = vertexFunc->shaderModule.get();
        vertexStage.pName = "main";

        GEVulkanFunction *fragmentFunc = (GEVulkanFunction *)desc.fragmentFunc;
        vk::PipelineShaderStageCreateInfo fragmentStage;
        fragmentStage.stage = vk::ShaderStageFlagBits::eFragment;
        fragmentStage.module = fragmentFunc->shaderModule.get();
        fragmentStage.pName = "main";
        
        std::initializer_list<vk::PipelineShaderStageCreateInfo> stages = {vertexStage,fragmentStage};
        
        pipeline_desc.pStages = stages.begin();
        pipeline_desc.stageCount = 2;

        vk::PipelineCacheCreateInfo info;
        info.initialDataSize = 0;
        info.pInitialData = nullptr;

        auto cache = device.createPipelineCacheUnique(info);
      
        return std::make_shared<GEVulkanRenderPipelineState>(pipeline_desc,pipeline_layout,cache);
    };
    SharedHandle<GEComputePipelineState> GEVulkanEngine::makeComputePipelineState(ComputePipelineDescriptor &desc){
        vk::PipelineLayoutCreateInfo layout_info;
        layout_info.pSetLayouts = nullptr;
        layout_info.setLayoutCount = 0;
        layout_info.pushConstantRangeCount = 0;
        layout_info.pPushConstantRanges = nullptr;

        vk::UniquePipelineLayout pipeline_layout = device.createPipelineLayoutUnique(layout_info);

        vk::ComputePipelineCreateInfo pipeline_desc;
        pipeline_desc.basePipelineIndex = -1;
        GEVulkanFunction *computeFunc = (GEVulkanFunction *)desc.computeFunc.get();
        vk::PipelineShaderStageCreateInfo computeStage;
        computeStage.stage = vk::ShaderStageFlagBits::eCompute;
        computeStage.module = computeFunc->shaderModule.get();
        computeStage.pName = "main";
        
        pipeline_desc.stage = computeStage;
        pipeline_desc.layout = pipeline_layout.get();
         auto pipeline = device.createComputePipelineUnique({},pipeline_desc);
         if(!VK_RESULT_SUCCEEDED(pipeline.result)){
            exit(1);
        };
        return std::make_shared<GEVulkanComputePipelineState>(pipeline.value,pipeline_layout);
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

    SharedHandle<GEFunctionLibrary> GEVulkanEngine::loadShaderLibrary(FS::Path path){

    };

    SharedHandle<GEFunctionLibrary> GEVulkanEngine::loadStdShaderLibrary(){
        
    };

_NAMESPACE_END_