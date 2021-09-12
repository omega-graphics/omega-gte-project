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


    struct  OMEGAGTE_EXPORT RenderPipelineDescriptor {
        SharedHandle<GTEShader> vertexFunc;
        SharedHandle<GTEShader> fragmentFunc;
        unsigned rasterSampleCount = 1;
        struct {
            bool enableDepth = false;
            bool enableStencil = false;
        } depthAndStencilDesc;
    };

    struct  OMEGAGTE_EXPORT ComputePipelineDescriptor {
        SharedHandle<GTEShader> computeFunc;

    };
    typedef class __GERenderPipelineState  GERenderPipelineState;
    typedef class __GEComputePipelineState GEComputePipelineState;


_NAMESPACE_END_

#endif
