#import "GEMetalCommandQueue.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"
#import "GEMetal.h"

#import <QuartzCore/QuartzCore.h>

_NAMESPACE_BEGIN_
    GEMetalCommandBuffer::GEMetalCommandBuffer(GEMetalCommandQueue *parentQueue):parentQueue(parentQueue),
    buffer({NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLCommandQueue>,parentQueue->commandQueue.handle()) commandBuffer]}){
       
    };

    void GEMetalCommandBuffer::startBlitPass(){
        buffer.assertExists();
        bp = [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) blitCommandEncoder];
    };

    void GEMetalCommandBuffer::finishBlitPass(){
        [bp endEncoding];
    };

    void GEMetalCommandBuffer::startRenderPass(const GERenderPassDescriptor & desc){
        buffer.assertExists();
        MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.renderTargetArrayLength = 1;
        if(desc.nRenderTarget){
            GEMetalNativeRenderTarget *n_rt = (GEMetalNativeRenderTarget *)desc.nRenderTarget;
            auto metalDrawable = n_rt->getDrawable();
            metalDrawable.assertExists();
            renderPassDesc.renderTargetWidth = n_rt->drawableSize.width;
            renderPassDesc.renderTargetHeight = n_rt->drawableSize.height;
            renderPassDesc.colorAttachments[0].texture = NSOBJECT_OBJC_BRIDGE(id<CAMetalDrawable>,metalDrawable.handle()).texture;
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
                auto & clearColor = desc.colorAttachment->clearColor;
                renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.r,clearColor.g,clearColor.b,clearColor.a);
                break;
            }
        }
        rp = [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) renderCommandEncoderWithDescriptor:renderPassDesc];
    };

    void GEMetalCommandBuffer::setRenderPipelineState(SharedHandle<GERenderPipelineState> & pipelineState){
        GEMetalRenderPipelineState *ps = (GEMetalRenderPipelineState *)pipelineState.get();
        ps->renderPipelineState.assertExists();
        [rp setRenderPipelineState:NSOBJECT_OBJC_BRIDGE(id<MTLRenderPipelineState>,ps->renderPipelineState.handle())];
    };

    void GEMetalCommandBuffer::setResourceConstAtVertexFunc(SharedHandle<GEBuffer> & buffer,unsigned index){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Vertex Func when not in render pass");
        GEMetalBuffer *metalBuffer = (GEMetalBuffer *)buffer.get();
        metalBuffer->metalBuffer.assertExists();
        [rp setVertexBuffer:NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,metalBuffer->metalBuffer.handle()) offset:0 atIndex:index];
    };

    void GEMetalCommandBuffer::setResourceConstAtVertexFunc(SharedHandle<GETexture> & texture,unsigned index){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Vertex Func when not in render pass");
        GEMetalTexture *metalTexture = (GEMetalTexture *)texture.get();
        [rp setVertexTexture:metalTexture->texture atIndex:index];
    };

    void GEMetalCommandBuffer::setResourceConstAtFragmentFunc(SharedHandle<GEBuffer> & buffer,unsigned index){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Fragment Func when not in render pass");
        GEMetalBuffer *metalBuffer = (GEMetalBuffer *)buffer.get();
        metalBuffer->metalBuffer.assertExists();
        [rp setFragmentBuffer:NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,metalBuffer->metalBuffer.handle()) offset:0 atIndex:index];
    };

    void GEMetalCommandBuffer::setResourceConstAtFragmentFunc(SharedHandle<GETexture> & texture,unsigned index){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Fragment Func when not in render pass");
        GEMetalTexture *metalTexture = (GEMetalTexture *)texture.get();
        [rp setFragmentTexture:metalTexture->texture atIndex:index];
    };

    void GEMetalCommandBuffer::setViewports(std::vector<GEViewport> viewports){
        std::vector<MTLViewport> metalViewports;
        auto viewports_it = viewports.begin();
        while(viewports_it != viewports.end()){
            GEViewport & viewport = *viewports_it;
            MTLViewport metalViewport;
            metalViewport.originX = viewport.x;
            metalViewport.originY = viewport.y;
            metalViewport.width = viewport.width;
            metalViewport.height = viewport.height;
            metalViewport.znear = viewport.nearDepth;
            metalViewport.zfar = viewport.farDepth;
            metalViewports.push_back(std::move(metalViewport));
            ++viewports_it;
        };
        [rp setViewports:metalViewports.data() count:metalViewports.size()];
    };

    void GEMetalCommandBuffer::setScissorRects(std::vector<GEScissorRect> scissorRects){
        std::vector<MTLScissorRect> metalRects;
        auto rects_it = scissorRects.begin();
        while(rects_it != scissorRects.end()){
            GEScissorRect & rect = *rects_it;
            MTLScissorRect metalRect;
            metalRect.x = rect.x;
            metalRect.y = rect.y;
            metalRect.width = rect.width;
            metalRect.height = rect.height;
            metalRects.push_back(std::move(metalRect));
            ++rects_it;
        };
        [rp setScissorRects:metalRects.data() count:metalRects.size()];
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
        buffer.assertExists();
        cp = [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) computeCommandEncoder];
    };

    void GEMetalCommandBuffer::setComputePipelineState(SharedHandle<GEComputePipelineState> & pipelineState){
        GEMetalComputePipelineState *ps = (GEMetalComputePipelineState *)pipelineState.get();
        ps->computePipelineState.assertExists();
        [cp setComputePipelineState:NSOBJECT_OBJC_BRIDGE(id<MTLComputePipelineState>,ps->computePipelineState.handle())];
    };

    void GEMetalCommandBuffer::finishComputePass(){

        [cp endEncoding];
    };

    void GEMetalCommandBuffer::__present_drawable(NSSmartPtr & drawable){
         buffer.assertExists();
         drawable.assertExists();
        [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) presentDrawable:
        NSOBJECT_OBJC_BRIDGE(id<CAMetalDrawable>,drawable.handle())];
    };
    
    void GEMetalCommandBuffer::__commit(){
         buffer.assertExists();
        [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) commit];
    };

    void GEMetalCommandBuffer::reset(){
        buffer = NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLCommandQueue>,parentQueue->commandQueue.handle()) commandBuffer]};
    };

    GEMetalCommandBuffer::~GEMetalCommandBuffer(){
        NSLog(@"Metal Command Buffer Destroy");
        buffer.assertExists();
        // [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) autorelease];
    };

    GEMetalCommandQueue::GEMetalCommandQueue(NSSmartPtr & queue,unsigned size):
    GECommandQueue(size),
    commandQueue(queue),commandBuffers(0){
        
    };

    void GEMetalCommandQueue::submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer){
        commandBuffers.push_back(commandBuffer);
    };

    void GEMetalCommandQueue::commitToGPU(){
        for(auto & commandBuffer : commandBuffers){
            auto mtlCommandBuffer = (GEMetalCommandBuffer *)commandBuffer.get();
            mtlCommandBuffer->__commit();
        };
        commandBuffers.clear();
    };

    void GEMetalCommandQueue::commitToGPUAndPresent(NSSmartPtr & drawable){
        auto & b = commandBuffers.back();
        ((GEMetalCommandBuffer *)b.get())->__present_drawable(drawable);
        for(auto & commandBuffer : commandBuffers){
            auto mtlCommandBuffer = (GEMetalCommandBuffer *)commandBuffer.get();
            mtlCommandBuffer->__commit();
        };
        commandBuffers.clear();
    };

    GEMetalCommandQueue::~GEMetalCommandQueue(){
        commandQueue.assertExists();
        NSLog(@"Metal Command Queue Destroy");
    //    [NSOBJECT_OBJC_BRIDGE(id<MTLCommandQueue>,commandQueue.handle()) autorelease];
    };

    SharedHandle<GECommandBuffer> GEMetalCommandQueue::getAvailableBuffer(){
        ++currentlyOccupied;
        auto s = this;
        return std::make_shared<GEMetalCommandBuffer>(s);
    };
_NAMESPACE_END_
