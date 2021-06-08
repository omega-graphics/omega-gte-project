#include "GTEBase.h"
#include <initializer_list>

#ifndef OMEGAGTE_GEPIPELINE_H
#define OMEGAGTE_GEPIPELINE_H

#if defined(TARGET_METAL) && defined(__OBJC__)
@protocol MTLLibrary;
#endif

_NAMESPACE_BEGIN_

    typedef struct __GEFunctionInternal GEFunction;

    struct GEFunctionLibrary {
        std::vector<SharedHandle<GEFunction>> functions;
    };

    struct OMEGAGTE_EXPORT InputAttributeDesc {
        typedef enum : int {
            FLOAT,
            FLOAT4,
            FLOAT3,
            FLOAT2,
            INT,
            INT2,
            INT3,
            INT4,
        } InputType;
        InputType type;
    };

    OMEGAGTE_EXPORT std::vector<InputAttributeDesc> ColoredVertexAttributes();
    OMEGAGTE_EXPORT std::vector<InputAttributeDesc> TexturedVertexAttributes();

    struct  OMEGAGTE_EXPORT RenderPipelineDescriptor {
        std::vector<InputAttributeDesc> vertexInputAttributes;
        SharedHandle<GEFunction> vertexFunc;
        SharedHandle<GEFunction> fragmentFunc;
    };

    struct  OMEGAGTE_EXPORT ComputePipelineDescriptor {
        std::vector<InputAttributeDesc> inputAttributes;
        SharedHandle<GEFunction> computeFunc;

    };
    typedef class __GERenderPipelineState  GERenderPipelineState;
    typedef class __GEComputePipelineState GEComputePipelineState;


_NAMESPACE_END_

#endif
