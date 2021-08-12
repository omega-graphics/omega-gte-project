#include "GTEBase.h"
#include "GTEShader.h"
#include <initializer_list>
#include <map>

#ifndef OMEGAGTE_GEPIPELINE_H
#define OMEGAGTE_GEPIPELINE_H

#if defined(TARGET_METAL) && defined(__OBJC__)
@protocol MTLLibrary;
#endif

_NAMESPACE_BEGIN_


    // #define STD_COLOREDVERTEX_FUNC "coloredVertexShader"
    // #define STD_FRAGMENTVERTEX_FUNC "coloredFragmentShader"

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

    // OMEGAGTE_EXPORT OmegaCommon::vector<InputAttributeDesc> ColoredVertexAttributes();
    // OMEGAGTE_EXPORT OmegaCommon::vector<InputAttributeDesc> TexturedVertexAttributes();

    struct  OMEGAGTE_EXPORT RenderPipelineDescriptor {
        OmegaCommon::Vector<InputAttributeDesc> vertexInputAttributes;
        SharedHandle<GTEShader> vertexFunc;
        SharedHandle<GTEShader> fragmentFunc;
    };

    struct  OMEGAGTE_EXPORT ComputePipelineDescriptor {
        std::vector<InputAttributeDesc> inputAttributes;
        SharedHandle<GTEShader> computeFunc;

    };
    typedef class __GERenderPipelineState  GERenderPipelineState;
    typedef class __GEComputePipelineState GEComputePipelineState;


_NAMESPACE_END_

#endif
