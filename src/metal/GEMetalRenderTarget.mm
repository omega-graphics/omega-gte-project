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
    auto & commandBuffers = commandQueue->commandBuffers;
    // [commandBuffers.back()->buffer presentDrawable:currentDrawable];
    for(auto buffer_it = commandBuffers.begin();buffer_it != commandBuffers.end();buffer_it++){     
        auto ref = *buffer_it;
        if(buffer_it == commandBuffers.end()-1)
            [ref->buffer presentDrawable:currentDrawable];
        [ref->buffer commit];
    };
    commandBuffers.clear();
};

void GEMetalNativeRenderTarget::reset(){
    currentDrawable = [metalLayer nextDrawable];
};


GEMetalTextureRenderTarget::GEMetalTextureRenderTarget(SharedHandle<GEMetalTexture> texture):texturePtr(texture){
    
};


SharedHandle<GERenderTarget::CommandBuffer> GEMetalTextureRenderTarget::commandBuffer(){
    return std::shared_ptr<GERenderTarget::CommandBuffer>(new GERenderTarget::CommandBuffer(this,GERenderTarget::CommandBuffer::Native,commandQueue->getAvailableBuffer()));
};

void GEMetalTextureRenderTarget::commit(){
    auto & commandBuffers = commandQueue->commandBuffers;
     for(auto commandBuffer : commandBuffers){
        [commandBuffer->buffer commit];
    };
};


_NAMESPACE_END_
