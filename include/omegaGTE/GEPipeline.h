#include "GTEBase.h"
#include "GTEShader.h"
#include <initializer_list>
#include <map>

#ifndef OMEGAGTE_GEPIPELINE_H
#define OMEGAGTE_GEPIPELINE_H

#ifdef None 
#undef None
#endif

#if defined(TARGET_METAL) && defined(__OBJC__)
@protocol MTLLibrary;
#endif

_NAMESPACE_BEGIN_

    enum class RasterCullMode : int {
        None = 0x00,
        Front,
        Back
    };

    enum class TriangleFillMode : int {
        Wireframe = 0x00,
        Solid
    };

    struct  OMEGAGTE_EXPORT RenderPipelineDescriptor {
        SharedHandle<GTEShader> vertexFunc;
        SharedHandle<GTEShader> fragmentFunc;
        unsigned rasterSampleCount = 0;
        RasterCullMode cullMode = RasterCullMode::None;
        TriangleFillMode triangleFillMode = TriangleFillMode::Solid;
        struct {
            bool enableDepth = false;
            bool enableStencil = false;
        } depthAndStencilDesc;
    };

    struct  OMEGAGTE_EXPORT ComputePipelineDescriptor {
        SharedHandle<GTEShader> computeFunc;

    };
    typedef struct __GERenderPipelineState  GERenderPipelineState;
    typedef struct __GEComputePipelineState GEComputePipelineState;


_NAMESPACE_END_

#endif
