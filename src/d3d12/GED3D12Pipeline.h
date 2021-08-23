#include "GED3D12.h"
#include "../GEPipeline.cpp"

#ifndef OMEGAGTE_D3D12_GED3D12PIPELINE_H
#define OMEGAGTE_D3D12_GED3D12PIPELINE_H

_NAMESPACE_BEGIN_

struct GED3D12Shader : public GTEShader {
    ComPtr<ID3DBlob> funcData;
    GED3D12Shader(ID3DBlob *blob):funcData(blob){};
};

class GED3D12RenderPipelineState : public __GERenderPipelineState {
public:
     ComPtr<ID3D12PipelineState> pipelineState;
    GED3D12RenderPipelineState(ID3D12PipelineState *state);
};

class GED3D12ComputePipelineState : public __GEComputePipelineState {
public: 
    ComPtr<ID3D12PipelineState> pipelineState;
    GED3D12ComputePipelineState(ID3D12PipelineState *state);
};

_NAMESPACE_END_

#endif