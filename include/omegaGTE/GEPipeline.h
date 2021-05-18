#include "GTEBase.h"

#ifndef OMEGAGTE_GEPIPELINE_H
#define OMEGAGTE_GEPIPELINE_H

_NAMESPACE_BEGIN_

    typedef struct __GEFunctionInternal GEFunction;

    struct  OMEGAGTE_EXPORT RenderPipelineDescriptor {
        SharedHandle<GEFunction> vertexFunc;
        SharedHandle<GEFunction> fragmentFunc;
    };

    struct  OMEGAGTE_EXPORT ComputePipelineDescriptor {
        SharedHandle<GEFunction> computeFunc;

    };
    typedef class __GERenderPipelineState  GERenderPipelineState;
    typedef class __GEComputePipelineState GEComputePipelineState;


_NAMESPACE_END_

#endif
