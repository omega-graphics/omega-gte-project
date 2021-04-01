#include "GEMetal.h"
#include "../GEPipeline.cpp"

#import <Metal/Metal.h>

#ifndef OMEGAGTE_METAL_GEMETALPIPELINE_H
#define OMEGAGTE_METAL_GEMETALPIPELINE_H

_NAMESPACE_BEGIN_

struct GEMetalFunction : public __GEFunctionInternal {
    id<MTLFunction> function;
};

class GEMetalRenderPipelineState : public __GERenderPipelineState {
public:
    id<MTLRenderPipelineState> renderPipelineState;
    GEMetalRenderPipelineState(id<MTLRenderPipelineState> renderPipelineState);
};

class GEMetalComputePipelineState : public __GEComputePipelineState {
public:
    id<MTLComputePipelineState> computePipelineState;
    GEMetalComputePipelineState(id<MTLComputePipelineState> computePipelineState);
};

_NAMESPACE_END_

#endif
