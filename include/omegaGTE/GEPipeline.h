#include "GTEBase.h"

#ifndef OMEGAGTE_GEPIPELINE_H
#define OMEGAGTE_GEPIPELINE_H

#if defined(TARGET_METAL) && defined(__OBJC__)
@protocol MTLLibrary;
#endif

_NAMESPACE_BEGIN_

    typedef struct __GEFunctionInternal GEFunction;

    #if defined(TARGET_METAL) && defined(__OBJC__)

    struct GEFunctionLibrary {
        id<MTLLibrary> mtlLib;
        std::vector<SharedHandle<GEFunction>> functions;
    };
    #endif

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
