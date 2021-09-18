#include "GEMetal.h"
#include "../GEPipeline.cpp"

#import <Metal/Metal.h>

#ifndef OMEGAGTE_METAL_GEMETALPIPELINE_H
#define OMEGAGTE_METAL_GEMETALPIPELINE_H

_NAMESPACE_BEGIN_

struct GEMetalShader : public GTEShader {
    NSSmartPtr library;
    NSSmartPtr function;
    GEMetalShader(NSSmartPtr & lib,NSSmartPtr & func);
};

class GEMetalRenderPipelineState : public __GERenderPipelineState {
public:
    NSSmartPtr renderPipelineState;
    GEMetalRenderPipelineState(SharedHandle<GTEShader> & _vertexShader,
                               SharedHandle<GTEShader> & _fragmentShader,
                               NSSmartPtr & renderPipelineState);
};

class GEMetalComputePipelineState : public __GEComputePipelineState {
public:
    NSSmartPtr computePipelineState;
    GEMetalComputePipelineState(SharedHandle<GTEShader> & _computeShader,
                                NSSmartPtr & computePipelineState);
};

_NAMESPACE_END_

#endif
