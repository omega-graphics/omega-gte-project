#import "GEMetalRenderTarget.h"
#import "GEMetalCommandQueue.h"

_NAMESPACE_BEGIN_

GEMetalNativeRenderTarget::GEMetalNativeRenderTarget(SharedHandle<GECommandQueue> commandQueue,CAMetalLayer *metalLayer):metalLayer(metalLayer),
commandQueue((GEMetalCommandQueue *)commandQueue.get()),drawableSize([metalLayer drawableSize]),currentDrawable([metalLayer nextDrawable]){
    
};

id<CAMetalDrawable> GEMetalNativeRenderTarget::getDrawable(){
    return currentDrawable;
};

SharedHandle<GERenderTarget::CommandBuffer> GEMetalNativeRenderTarget::commandBuffer(){
    return std::shared_ptr<GERenderTarget::CommandBuffer>(new GERenderTarget::CommandBuffer(this,GERenderTarget::CommandBuffer::Native,commandQueue->getAvailableBuffer()));
};


void GEMetalNativeRenderTarget::commitAndPresent(){
    commandQueue->commitToGPUAndPresent(currentDrawable);
};

void GEMetalNativeRenderTarget::reset(){
    currentDrawable = [metalLayer nextDrawable];
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
