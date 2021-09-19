#include "GED3D12.h"
#include "../GEPipeline.cpp"

#ifndef OMEGAGTE_D3D12_GED3D12PIPELINE_H
#define OMEGAGTE_D3D12_GED3D12PIPELINE_H

_NAMESPACE_BEGIN_

struct GED3D12Shader : public GTEShader {
public:
    D3D12_SHADER_BYTECODE shaderBytecode;
};

class GED3D12RenderPipelineState : public __GERenderPipelineState {
public:
     ComPtr<ID3D12PipelineState> pipelineState;
     ComPtr<ID3D12RootSignature> rootSignature;
    GED3D12RenderPipelineState(ID3D12PipelineState *state,ID3D12RootSignature *signature);
};

class GED3D12ComputePipelineState : public __GEComputePipelineState {
public: 
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12RootSignature> rootSignature;
    GED3D12ComputePipelineState(ID3D12PipelineState *state,ID3D12RootSignature *signature);
};

_NAMESPACE_END_

#endif