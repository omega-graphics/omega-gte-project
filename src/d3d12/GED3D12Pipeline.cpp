#include "GED3D12Pipeline.h"

_NAMESPACE_BEGIN_


GED3D12RenderPipelineState::GED3D12RenderPipelineState(ID3D12PipelineState *state,ID3D12RootSignature *signature):pipelineState(state),rootSignature(signature){};

GED3D12ComputePipelineState::GED3D12ComputePipelineState(ID3D12PipelineState *state,ID3D12RootSignature *signature):pipelineState(state),rootSignature(signature){};

_NAMESPACE_END_