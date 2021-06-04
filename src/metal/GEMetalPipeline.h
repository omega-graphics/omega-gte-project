#include "GEMetal.h"
#include "../GEPipeline.cpp"

#import <Metal/Metal.h>

#ifndef OMEGAGTE_METAL_GEMETALPIPELINE_H
#define OMEGAGTE_METAL_GEMETALPIPELINE_H

_NAMESPACE_BEGIN_

struct GEMetalFunction : public __GEFunctionInternal {
    NSSmartPtr function;
    GEMetalFunction(NSSmartPtr & func);
};

class GEMetalRenderPipelineState : public __GERenderPipelineState {
public:
    NSSmartPtr renderPipelineState;
    GEMetalRenderPipelineState(NSSmartPtr & renderPipelineState);
};

class GEMetalComputePipelineState : public __GEComputePipelineState {
public:
    NSSmartPtr computePipelineState;
    GEMetalComputePipelineState(NSSmartPtr & computePipelineState);
};

_NAMESPACE_END_

#endif
