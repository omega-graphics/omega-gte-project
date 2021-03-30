#include "GTEBase.h"

#ifndef OMEGAGTE_GEPIPELINE_H
#define OMEGAGTE_GEPIPELINE_H

_NAMESPACE_BEGIN_

    typedef struct __GEFunctionInternal GEFunction;

    struct RenderPipelineDescriptor {
        SharedHandle<GEFunction> vertexFunc;
        SharedHandle<GEFunction> fragmentFunc;
    };

    struct ComputePipelineDescriptor {
        SharedHandle<GEFunction> computeFunc;

    };
    typedef class __GERenderPipelineState  GERenderPipelineState;
    typedef class __GEComputePipelineState GEComputePipelineState;


_NAMESPACE_END_

#endif
