#import "GEMetalRenderTarget.h"
#include "GEMetal.h"
#import "GEMetalCommandQueue.h"

_NAMESPACE_BEGIN_

GEMetalNativeRenderTarget::GEMetalNativeRenderTarget(SharedHandle<GECommandQueue> commandQueue,CAMetalLayer *metalLayer):metalLayer(metalLayer),
commandQueue(commandQueue),drawableSize([metalLayer drawableSize]),currentDrawable({NSOBJECT_CPP_BRIDGE [metalLayer nextDrawable]}){
    
};

NSSmartPtr & GEMetalNativeRenderTarget::getDrawable(){
    return currentDrawable;
};

SharedHandle<GERenderTarget::CommandBuffer> GEMetalNativeRenderTarget::commandBuffer(){
    return std::shared_ptr<GERenderTarget::CommandBuffer>(new GERenderTarget::CommandBuffer(this,GERenderTarget::CommandBuffer::Native,commandQueue->getAvailableBuffer()));
};


void GEMetalNativeRenderTarget::commitAndPresent(){
    auto mtlqueue = (GEMetalCommandQueue *)commandQueue.get();
    mtlqueue->commitToGPUAndPresent(currentDrawable);
};

void GEMetalNativeRenderTarget::reset(){
    currentDrawable = NSObjectHandle{ NSOBJECT_CPP_BRIDGE [metalLayer nextDrawable]};
};

void GEMetalNativeRenderTarget::submitCommandBuffer(SharedHandle<CommandBuffer> &commandBuffer){
    if(commandBuffer->commandBuffer)
        commandQueue->submitCommandBuffer(commandBuffer->commandBuffer);
};


GEMetalTextureRenderTarget::GEMetalTextureRenderTarget(SharedHandle<GEMetalTexture> texture):texturePtr(texture){
    
};


SharedHandle<GERenderTarget::CommandBuffer> GEMetalTextureRenderTarget::commandBuffer(){
    return std::shared_ptr<GERenderTarget::CommandBuffer>(new GERenderTarget::CommandBuffer(this,GERenderTarget::CommandBuffer::Native,commandQueue->getAvailableBuffer()));
};

void GEMetalTextureRenderTarget::commit(){
    commandQueue->commitToGPU();
};


_NAMESPACE_END_
