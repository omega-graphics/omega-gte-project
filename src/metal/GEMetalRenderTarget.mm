#import "GEMetalRenderTarget.h"

_NAMESPACE_BEGIN_

GEMetalNativeRenderTarget::GEMetalNativeRenderTarget(CAMetalLayer *metalLayer):metalLayer(metalLayer),currentDrawable([metalLayer nextDrawable]){
    
};

SharedHandle<GERenderTarget::CommandBuffer> GEMetalNativeRenderTarget::commandBuffer(){
    return std::shared_ptr<GERenderTarget::CommandBuffer>(new GERenderTarget::CommandBuffer(this,GERenderTarget::CommandBuffer::Native,commandQueue->getAvailableBuffer()));
};


void GEMetalNativeRenderTarget::commitAndPresent(){
    @autoreleasepool {
        NSMutableArray<id<MTLCommandBuffer>> *commandBuffers = commandQueue->commandBuffers;
        [[commandBuffers lastObject] presentDrawable:currentDrawable];
        for(id<MTLCommandBuffer> cb in commandBuffers){
            [cb commit];
        };
    }
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
    @autoreleasepool {
        NSMutableArray<id<MTLCommandBuffer>> *commandBuffers = commandQueue->commandBuffers;
        for(id<MTLCommandBuffer> cb in commandBuffers){
            [cb commit];
        };
    }
};


_NAMESPACE_END_
