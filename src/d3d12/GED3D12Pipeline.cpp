#include "GED3D12Pipeline.h"

_NAMESPACE_BEGIN_

GED3D12RenderPipelineState::GED3D12RenderPipelineState(ID3D12PipelineState *state):pipelineState(state){};

GED3D12ComputePipelineState::GED3D12ComputePipelineState(ID3D12PipelineState *state):pipelineState(state){};

_NAMESPACE_END_