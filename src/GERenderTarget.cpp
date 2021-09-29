#include "omegaGTE/GERenderTarget.h"
#include "omegaGTE/GTEBase.h"
#include "omegaGTE/GECommandQueue.h"

_NAMESPACE_BEGIN_

GERenderTarget::RenderPassDesc::ColorAttachment::ClearColor::ClearColor(float r,float g,float b,float a):r(r),g(g),b(b),a(a){
    
};

GERenderTarget::RenderPassDesc::ColorAttachment::ColorAttachment(GERenderTarget::RenderPassDesc::ColorAttachment::ClearColor clearColor,GERenderTarget::RenderPassDesc::ColorAttachment::LoadAction loadAction):clearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a),loadAction(loadAction){
    
};

GERenderTarget::CommandBuffer::CommandBuffer(GERenderTarget *renderTarget,GERTType type,SharedHandle<GECommandBuffer> commandBuffer):
renderTargetPtr(renderTarget),
renderTargetTy(type),
commandBuffer(std::move(commandBuffer)){
    
};

void GERenderTarget::CommandBuffer::startRenderPass(const GERenderTarget::RenderPassDesc & desc){
    GERenderPassDescriptor renderPassDesc;
    if(renderTargetTy == Native){
        renderPassDesc.nRenderTarget = (GENativeRenderTarget *)renderTargetPtr;
    }
    else if(renderTargetTy == Texture){
        renderPassDesc.tRenderTarget = (GETextureRenderTarget *)renderTargetPtr;
    };
    renderPassDesc.colorAttachment = desc.colorAttachment;
    
    commandBuffer->startRenderPass(renderPassDesc);
};

void GERenderTarget::CommandBuffer::setRenderPipelineState(SharedHandle<GERenderPipelineState> &pipelineState){
    commandBuffer->setRenderPipelineState(pipelineState);
};

void GERenderTarget::CommandBuffer::bindResourceAtVertexShader(SharedHandle<GEBuffer> & buffer,unsigned id){
    commandBuffer->bindResourceAtVertexShader(buffer,id);
};

void GERenderTarget::CommandBuffer::bindResourceAtVertexShader(SharedHandle<GETexture> & texture,unsigned id){
    commandBuffer->bindResourceAtVertexShader(texture,id);
};

void GERenderTarget::CommandBuffer::bindResourceAtFragmentShader(SharedHandle<GEBuffer> & buffer,unsigned id){
    commandBuffer->bindResourceAtFragmentShader(buffer,id);
};

void GERenderTarget::CommandBuffer::bindResourceAtFragmentShader(SharedHandle<GETexture> & texture,unsigned id){
    commandBuffer->bindResourceAtFragmentShader(texture,id);
};

void GERenderTarget::CommandBuffer::drawPolygons(RenderPassDrawPolygonType polygonType,unsigned vertexCount,size_t start){
    commandBuffer->drawPolygons(polygonType,vertexCount,start);
};

void GERenderTarget::CommandBuffer::setViewports(std::vector<GEViewport> viewports) {
    commandBuffer->setViewports(viewports);
}

void GERenderTarget::CommandBuffer::setScissorRects(std::vector<GEScissorRect> scissorRects) {
    commandBuffer->setScissorRects(scissorRects);
}

void GERenderTarget::CommandBuffer::endRenderPass(){
    commandBuffer->finishRenderPass();
};

void GERenderTarget::CommandBuffer::startComputePass(SharedHandle<GEComputePipelineState> & pipelineState){
    GEComputePassDescriptor comp;
    commandBuffer->startComputePass(comp);
};

void GERenderTarget::CommandBuffer::endComputePass(){
    commandBuffer->finishComputePass();
};

// void GERenderTarget::CommandBuffer::schedule(){
//     commandBuffer->commitToQueue();
// };

void GERenderTarget::CommandBuffer::reset(){
    commandBuffer->reset();
};





_NAMESPACE_END_
