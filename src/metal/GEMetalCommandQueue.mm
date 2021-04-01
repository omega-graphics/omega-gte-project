#import "GEMetalCommandQueue.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"

#import <QuartzCore/QuartzCore.h>
_NAMESPACE_BEGIN_
    GEMetalCommandBuffer::GEMetalCommandBuffer(id<MTLCommandBuffer> buffer,GEMetalCommandQueue *parentQueue):buffer(buffer),parentQueue(parentQueue){

    };

    void GEMetalCommandBuffer::startRenderPass(const GERenderPassDescriptor & desc){
        MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.renderTargetArrayLength = 1;
        if(desc.nRenderTarget){
            GEMetalNativeRenderTarget *n_rt = (GEMetalNativeRenderTarget *)desc.nRenderTarget;
            id<CAMetalDrawable> metalDrawable = n_rt->currentDrawable;
            renderPassDesc.renderTargetWidth = n_rt->drawableSize->width;
            renderPassDesc.renderTargetHeight = n_rt->drawableSize->height;
            renderPassDesc.colorAttachments[0].texture = metalDrawable.texture;
        }
        else if(desc.tRenderTarget){
            GEMetalTextureRenderTarget *t_rt = (GEMetalTextureRenderTarget *)desc.tRenderTarget;
            renderPassDesc.renderTargetWidth = t_rt->texturePtr->desc.width;
            renderPassDesc.renderTargetHeight = t_rt->texturePtr->desc.height;
            renderPassDesc.colorAttachments[0].texture = t_rt->texturePtr->texture;
        }
        else {
            DEBUG_STREAM("Failed to Create GERenderPass");
            exit(1);
        };
        
        switch (desc.colorAttachment->loadAction) {
            case GERenderPassDescriptor::ColorAttachment::Load : {
                renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
                renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionDontCare;
                break;
            }
            case GERenderPassDescriptor::ColorAttachment::LoadPreserve : {
                renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
                renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
                break;
            }
            case GERenderPassDescriptor::ColorAttachment::Discard : {
                renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
                renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionDontCare;
                break;
            }
            case GERenderPassDescriptor::ColorAttachment::Clear : {
                renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
                renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
                break;
            }
        }
        rp = [buffer renderCommandEncoderWithDescriptor:renderPassDesc];
    };

    void GEMetalCommandBuffer::setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState){
        GEMetalRenderPipelineState *ps = (GEMetalRenderPipelineState *)pipelineState.get();
        [rp setRenderPipelineState:ps->renderPipelineState];
    };

    void GEMetalCommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t startIdx){
        assert((rp && (cp == nil)) && "Cannot Draw Polygons when not in render pass");
        MTLPrimitiveType primativeType;
        if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::Triangle){
            primativeType = MTLPrimitiveTypeTriangle;
        }
        else if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::TriangleStrip){
            primativeType = MTLPrimitiveTypeTriangleStrip;
        };
        [rp drawPrimitives:primativeType vertexStart:startIdx vertexCount:vertexCount instanceCount:1];
    };

    void GEMetalCommandBuffer::finishRenderPass(){
        [rp endEncoding];
    };

    void GEMetalCommandBuffer::startComputePass(const GEComputePassDescriptor & desc){
        cp = [buffer computeCommandEncoder];
    };

    void GEMetalCommandBuffer::setComputePipelineState(SharedHandle<GEComputePipelineState> & pipelineState){
        GEMetalComputePipelineState *ps = (GEMetalComputePipelineState *)pipelineState.get();
        [cp setComputePipelineState:ps->computePipelineState];
    };

    void GEMetalCommandBuffer::finishComputePass(){
        [cp endEncoding];
    };
    
    void GEMetalCommandBuffer::commitToQueue(){
        [buffer enqueue];
        [parentQueue->commandBuffers addObject:buffer];
    };

    GEMetalCommandQueue::GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size):GECommandQueue(size),commandQueue(queue){
        commandBuffers = [[NSMutableArray alloc] init];
    };

    void GEMetalCommandQueue::commitToGPU(){
        @autoreleasepool {
            for(id<MTLCommandBuffer> commandBuffer in commandBuffers){
                [commandBuffer commit];
                // [commandBuffer waitUntilScheduled];
            };
        };
    };

    GEMetalCommandQueue::~GEMetalCommandQueue(){
        [commandBuffers release];
    };

    SharedHandle<GECommandBuffer> GEMetalCommandQueue::getAvailableBuffer(){
        ++currentlyOccupied;
        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
        auto s = this;
        return std::make_shared<GEMetalCommandBuffer>(commandBuffer,s);
    };
_NAMESPACE_END_
