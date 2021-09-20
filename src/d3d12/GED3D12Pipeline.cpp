#include "GED3D12Pipeline.h"

_NAMESPACE_BEGIN_


GED3D12RenderPipelineState::GED3D12RenderPipelineState(SharedHandle<GTEShader> & _vertShader,SharedHandle<GTEShader> & _fragShader,ID3D12PipelineState *state,ID3D12RootSignature *signature): __GERenderPipelineState(_vertShader,_fragShader),pipelineState(state),rootSignature(signature){};

GED3D12ComputePipelineState::GED3D12ComputePipelineState(SharedHandle<GTEShader> & _shader,ID3D12PipelineState *state,ID3D12RootSignature *signature): __GEComputePipelineState(_shader),pipelineState(state),rootSignature(signature){};

_NAMESPACE_END_