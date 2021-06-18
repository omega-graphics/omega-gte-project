#include "../GEPipeline.cpp"
#include "GED3D12.h"

#ifndef OMEGAGTE_D3D12_GED3D12PIPELINE_H
#define OMEGAGTE_D3D12_GED3D12PIPELINE_H

_NAMESPACE_BEGIN_

struct GED3D12Function : public __GEFunctionInternal {
    ComPtr<ID3DBlob> funcData;
    GED3D12Function(ID3DBlob *blob);
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