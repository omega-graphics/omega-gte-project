#import "GEMetalPipeline.h"

_NAMESPACE_BEGIN_

GEMetalRenderPipelineState::GEMetalRenderPipelineState(id<MTLRenderPipelineState> renderPipelineState):renderPipelineState(renderPipelineState){
    
};

GEMetalComputePipelineState::GEMetalComputePipelineState(id<MTLComputePipelineState> computePipelineState):computePipelineState(computePipelineState){
    
};

_NAMESPACE_END_
