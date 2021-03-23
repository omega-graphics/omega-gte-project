#include "GTEBase.h"

#ifndef OMEGAGTE_GEPIPELINE_H
#define OMEGAGTE_GEPIPELINE_H

_NAMESPACE_BEGIN_

    typedef struct __GEFunctionInternal GEFunction;

    struct RenderPipelineDescriptor {
        
    };

    struct ComputePipelineDescriptor {
        GEFunction *computeFunc;

    };

    class GERenderPipelineState {
        GEFunction *vertexFunc;
        GEFunction *fragmentFunc;
    };

    class GEComputePipelineState {

    };
_NAMESPACE_END_

#endif
