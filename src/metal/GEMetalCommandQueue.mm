#import "GEMetalCommandQueue.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"
#import "GEMetal.h"

#include <cstdlib>

#import <QuartzCore/QuartzCore.h>

_NAMESPACE_BEGIN_
    GEMetalCommandBuffer::GEMetalCommandBuffer(GEMetalCommandQueue *parentQueue):parentQueue(parentQueue),
    buffer({NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLCommandQueue>,parentQueue->commandQueue.handle()) commandBuffer]}){
       
    };

    unsigned GEMetalCommandBuffer::getResourceLocalIndexFromGlobalIndex(unsigned _id,omegasl_shader & shader){
        OmegaCommon::ArrayRef<omegasl_shader_layout_desc> descArr {shader.pLayout,shader.pLayout + shader.nLayout};
        for(auto l : descArr){
            if(l.location == _id){
                return l.gpu_relative_loc;
            }
        }
        return -1;
    };

    void GEMetalCommandBuffer::startBlitPass(){
        buffer.assertExists();
        bp = [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) blitCommandEncoder];
    };

    void GEMetalCommandBuffer::copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest) {
        assert(bp && "Must be in BLIT PASS");
        [bp copyFromTexture:
                NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,((GEMetalTexture *)src.get())->texture.handle())
                  toTexture:
        NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,((GEMetalTexture *)dest.get())->texture.handle())];
    }

    void GEMetalCommandBuffer::copyTextureToTexture(SharedHandle<GETexture> &src, SharedHandle<GETexture> &dest,
                                                    const TextureRegion &region, const GPoint3D &destCoord) {
        assert(bp && "Must be in BLIT PASS");
        auto mtl_src_texture = (GEMetalTexture *)src.get();
        auto mtl_dest_texture = (GEMetalTexture *)dest.get();
        [bp copyFromTexture: NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,mtl_src_texture->texture.handle())
                sourceSlice:0 sourceLevel:0
                  toTexture: NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,mtl_dest_texture->texture.handle())
           destinationSlice:0 destinationLevel:0 sliceCount:1 levelCount:
                        NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,mtl_src_texture->texture.handle()).mipmapLevelCount];
    }

    void GEMetalCommandBuffer::finishBlitPass(){
        [bp endEncoding];
    };

    void GEMetalCommandBuffer::setVertexBuffer(SharedHandle<GEBuffer> &buffer) {

    }

    void GEMetalCommandBuffer::startRenderPass(const GERenderPassDescriptor & desc){
        buffer.assertExists();
        MTLRenderPassDescriptor *renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.renderTargetArrayLength = 1;

        id<MTLTexture> multiSampleTextureTarget = nil;

        if(desc.nRenderTarget){
            auto *n_rt = (GEMetalNativeRenderTarget *)desc.nRenderTarget;
            auto metalDrawable = n_rt->getDrawable();
            metalDrawable.assertExists();
            renderPassDesc.renderTargetWidth = n_rt->drawableSize.width;
            renderPassDesc.renderTargetHeight = n_rt->drawableSize.height;
            id<MTLTexture> renderTarget;
            if(desc.multisampleResolve){
                renderTarget = NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,((GEMetalTexture *)desc.resolveDesc.multiSampleTextureSrc.get())->texture.handle());
                multiSampleTextureTarget = NSOBJECT_OBJC_BRIDGE(id<CAMetalDrawable>,metalDrawable.handle()).texture;
            }
            else {
                renderTarget =  NSOBJECT_OBJC_BRIDGE(id<CAMetalDrawable>,metalDrawable.handle()).texture;
            }
            renderPassDesc.colorAttachments[0].texture =renderTarget;
        }
        else if(desc.tRenderTarget){
            auto *t_rt = (GEMetalTextureRenderTarget *)desc.tRenderTarget;
            renderPassDesc.renderTargetWidth = t_rt->texturePtr->desc.width;
            renderPassDesc.renderTargetHeight = t_rt->texturePtr->desc.height;
            id<MTLTexture> renderTarget;
            if(desc.multisampleResolve){
                renderTarget = NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,((GEMetalTexture *)desc.resolveDesc.multiSampleTextureSrc.get())->texture.handle());
                multiSampleTextureTarget =  NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,t_rt->texturePtr->texture.handle());
            }
            else {
                renderTarget =  NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,t_rt->texturePtr->texture.handle());
            }
            renderPassDesc.colorAttachments[0].texture = renderTarget;
        }
        else {
            DEBUG_STREAM("Failed to Create GERenderPass");
            exit(1);
        };

        if(desc.multisampleResolve){
            auto resolveTexture = (GEMetalTexture *)desc.resolveDesc.multiSampleTextureSrc.get();
            renderPassDesc.colorAttachments[0].resolveTexture = multiSampleTextureTarget;
            renderPassDesc.colorAttachments[0].resolveSlice = desc.resolveDesc.slice;
            renderPassDesc.colorAttachments[0].resolveDepthPlane = desc.resolveDesc.depth;
            renderPassDesc.colorAttachments[0].resolveLevel = desc.resolveDesc.level;
        }
        
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
        auto *ps = (GEMetalRenderPipelineState *)pipelineState.get();
        ps->renderPipelineState.assertExists();
        [rp setRenderPipelineState:NSOBJECT_OBJC_BRIDGE(id<MTLRenderPipelineState>,ps->renderPipelineState.handle())];
        renderPipelineState = ps;
    };

    void GEMetalCommandBuffer::bindResourceAtVertexShader(SharedHandle<GEBuffer> & buffer,unsigned _id){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Vertex Func when not in render pass");
        GEMetalBuffer *metalBuffer = (GEMetalBuffer *)buffer.get();
        metalBuffer->metalBuffer.assertExists();
        unsigned index = getResourceLocalIndexFromGlobalIndex(_id,renderPipelineState->vertexShader->internal);
        [rp setVertexBuffer:NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,metalBuffer->metalBuffer.handle()) offset:0 atIndex:index];
    };

    void GEMetalCommandBuffer::bindResourceAtVertexShader(SharedHandle<GETexture> & texture,unsigned _id){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Vertex Func when not in render pass");
        GEMetalTexture *metalTexture = (GEMetalTexture *)texture.get();
        unsigned index = getResourceLocalIndexFromGlobalIndex(_id,renderPipelineState->vertexShader->internal);
        [rp setVertexTexture:NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,metalTexture->texture.handle()) atIndex:index];
    };

    void GEMetalCommandBuffer::bindResourceAtFragmentShader(SharedHandle<GEBuffer> & buffer,unsigned _id){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Fragment Func when not in render pass");
        auto *metalBuffer = (GEMetalBuffer *)buffer.get();
        metalBuffer->metalBuffer.assertExists();
        unsigned index = getResourceLocalIndexFromGlobalIndex(_id,renderPipelineState->fragmentShader->internal);
        [rp setFragmentBuffer:NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,metalBuffer->metalBuffer.handle()) offset:0 atIndex:index];
    };

    void GEMetalCommandBuffer::bindResourceAtFragmentShader(SharedHandle<GETexture> & texture,unsigned _id){
        assert((rp && (cp == nil)) && "Cannot Resource Const on a Fragment Func when not in render pass");
        auto *metalTexture = (GEMetalTexture *)texture.get();
        unsigned index = getResourceLocalIndexFromGlobalIndex(_id,renderPipelineState->fragmentShader->internal);
        [rp setFragmentTexture:NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,metalTexture->texture.handle()) atIndex:index];
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
            metalViewports.push_back(metalViewport);
            ++viewports_it;
        };
        auto s = metalViewports.size();
        [rp setViewports:metalViewports.data() count:s];
    };

    void GEMetalCommandBuffer::setScissorRects(std::vector<GEScissorRect> scissorRects){
        std::vector<MTLScissorRect> metalRects;
        auto rects_it = scissorRects.begin();
        while(rects_it != scissorRects.end()){
            GEScissorRect & rect = *rects_it;
            MTLScissorRect metalRect;
            metalRect.x = (NSUInteger)rect.x;
            metalRect.y = (NSUInteger)rect.y;
            metalRect.width = (NSUInteger)rect.width;
            metalRect.height = (NSUInteger)rect.height;
            metalRects.push_back(metalRect);
            ++rects_it;
        };
        auto s = metalRects.size();
        [rp setScissorRects:metalRects.data() count:s];
    };
    
    void GEMetalCommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t startIdx){
        assert((rp && (cp == nil)) && "Cannot Draw Polygons when not in render pass");
        MTLPrimitiveType primativeType;
        if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::Triangle){
            primativeType = MTLPrimitiveTypeTriangle;
        }
        else if(polygonType == GECommandBuffer::RenderPassDrawPolygonType::TriangleStrip){
            primativeType = MTLPrimitiveTypeTriangleStrip;
        }
        else {
            primativeType = MTLPrimitiveTypeTriangle;
        };

        NSLog(@"CALLING DRAW PRIMITIVES");
        [rp drawPrimitives:primativeType vertexStart:startIdx vertexCount:vertexCount];
    };

    void GEMetalCommandBuffer::finishRenderPass(){
        renderPipelineState = nullptr;
        [rp endEncoding];
    };

    void GEMetalCommandBuffer::startComputePass(const GEComputePassDescriptor & desc){
        buffer.assertExists();
        cp = [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) computeCommandEncoder];
    };

    void GEMetalCommandBuffer::setComputePipelineState(SharedHandle<GEComputePipelineState> & pipelineState){
        assert(cp != nil && "");
        auto * ps = (GEMetalComputePipelineState *)pipelineState.get();
        ps->computePipelineState.assertExists();
        computePipelineState = ps;
        [cp setComputePipelineState:NSOBJECT_OBJC_BRIDGE(id<MTLComputePipelineState>,ps->computePipelineState.handle())];
    };

    void GEMetalCommandBuffer::bindResourceAtComputeShader(SharedHandle<GEBuffer> &buffer, unsigned int _id) {
        assert(cp != nil && "");
        auto mtl_buffer = (GEMetalBuffer *)buffer.get();
        [cp setBuffer:NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,mtl_buffer->metalBuffer.handle()) offset:0 atIndex:getResourceLocalIndexFromGlobalIndex(_id,computePipelineState->computeShader->internal)];
    }

    void GEMetalCommandBuffer::bindResourceAtComputeShader(SharedHandle<GETexture> &texture, unsigned int _id) {
        assert(cp != nil && "");
        auto mtl_texture = (GEMetalTexture *)texture.get();
        [cp setTexture:NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,mtl_texture->texture.handle()) atIndex:getResourceLocalIndexFromGlobalIndex(_id,computePipelineState->computeShader->internal)];
    }

    void GEMetalCommandBuffer::dispatchThreads(unsigned int x, unsigned int y, unsigned int z) {
        assert(cp != nil && "");
        auto & threadgroup_desc = computePipelineState->computeShader->internal.threadgroupDesc;
        [cp dispatchThreads:MTLSizeMake(x,y,z) threadsPerThreadgroup:MTLSizeMake(threadgroup_desc.x,threadgroup_desc.y,threadgroup_desc.z)];
    }

    void GEMetalCommandBuffer::finishComputePass(){
        [cp endEncoding];
        computePipelineState = nullptr;
    };

    void GEMetalCommandBuffer::_present_drawable(NSSmartPtr & drawable){
         buffer.assertExists();
         drawable.assertExists();
        [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) presentDrawable:
        NSOBJECT_OBJC_BRIDGE(id<CAMetalDrawable>,drawable.handle())];
        NSLog(@"Present Drawable");
    };
    
    void GEMetalCommandBuffer::_commit(){
         buffer.assertExists();
         [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer){
            if(commandBuffer.status == MTLCommandBufferStatusError){
                NSLog(@"Command Buffer Failed to Execute. Error: %@",commandBuffer.error);
            }
            else if(commandBuffer.status == MTLCommandBufferStatusCompleted){
                NSLog(@"Successfully completed Command Buffer!");
            }
         }];
        [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) commit];
        NSLog(@"Commit to GPU!");
    };

    void GEMetalCommandBuffer::waitForFence(SharedHandle<GEFence> &fence, unsigned int val) {
        auto event = (GEMetalFence *)fence.get();
        [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle())
                encodeWaitForEvent:NSOBJECT_OBJC_BRIDGE(id<MTLEvent>,event->metalEvent.handle()) value:val];
    }

    void GEMetalCommandBuffer::signalFence(SharedHandle<GEFence> &fence, unsigned int val) {
        auto event = (GEMetalFence *)fence.get();
        [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle())
                encodeSignalEvent:NSOBJECT_OBJC_BRIDGE(id<MTLEvent>,event->metalEvent.handle()) value:val];
    }

    void GEMetalCommandBuffer::reset(){
        buffer = NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLCommandQueue>,parentQueue->commandQueue.handle()) commandBuffer]};
    };

    GEMetalCommandBuffer::~GEMetalCommandBuffer(){
        NSLog(@"Metal Command Buffer Destroy");
        buffer.assertExists();
        // [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,buffer.handle()) autorelease];
    }

    GEMetalCommandQueue::GEMetalCommandQueue(NSSmartPtr & queue,unsigned size):
    GECommandQueue(size),
    commandQueue(queue),commandBuffers(){
        
    };

    void GEMetalCommandQueue::submitCommandBuffer(SharedHandle<GECommandBuffer> &commandBuffer){
        auto _commandBuffer = (GEMetalCommandBuffer *)commandBuffer.get();
        [NSOBJECT_OBJC_BRIDGE(id<MTLCommandBuffer>,_commandBuffer->buffer.handle()) enqueue];
        commandBuffers.push_back(commandBuffer);
    };

    void GEMetalCommandQueue::commitToGPU(){
        for(auto & commandBuffer : commandBuffers){
            auto mtlCommandBuffer = (GEMetalCommandBuffer *)commandBuffer.get();
            mtlCommandBuffer->_commit();
        };
//        commandBuffers.clear();
    };

    void GEMetalCommandQueue::commitToGPUAndPresent(NSSmartPtr & drawable){
        auto & b = commandBuffers.back();
        ((GEMetalCommandBuffer *)b.get())->_present_drawable(drawable);
        for(auto & commandBuffer : commandBuffers){
            auto mtlCommandBuffer = (GEMetalCommandBuffer *)commandBuffer.get();
            mtlCommandBuffer->_commit();
        };
//        commandBuffers.clear();
    };

    GEMetalCommandQueue::~GEMetalCommandQueue(){
        commandQueue.assertExists();
        NSLog(@"Metal Command Queue Destroy");
    //    [NSOBJECT_OBJC_BRIDGE(id<MTLCommandQueue>,commandQueue.handle()) autorelease];
    };

    SharedHandle<GECommandBuffer> GEMetalCommandQueue::getAvailableBuffer(){
        ++currentlyOccupied;
        auto s = this;
        return SharedHandle<GECommandBuffer>(new GEMetalCommandBuffer(s));
    };
_NAMESPACE_END_
