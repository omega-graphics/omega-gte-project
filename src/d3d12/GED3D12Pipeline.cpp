#include "GED3D12Pipeline.h"

_NAMESPACE_BEGIN_


GED3D12RenderPipelineState::GED3D12RenderPipelineState(SharedHandle<GTEShader> & _vertShader,
                                                       SharedHandle<GTEShader> & _fragShader,
                                                       ID3D12PipelineState *state,
                                                       ID3D12RootSignature *signature,
                                                       D3D12_ROOT_SIGNATURE_DESC1 & rootSignatureDesc):
                                                       __GERenderPipelineState(_vertShader,_fragShader),
                                                       pipelineState(state),
                                                       rootSignature(signature),rootSignatureDesc(rootSignatureDesc){};

GED3D12RenderPipelineState::~GED3D12RenderPipelineState() {
    delete [] rootSignatureDesc.pParameters;
    delete [] rootSignatureDesc.pStaticSamplers;
}

GED3D12ComputePipelineState::GED3D12ComputePipelineState(SharedHandle<GTEShader> & _shader,
                                                         ID3D12PipelineState *state,
                                                         ID3D12RootSignature *signature,
                                                         D3D12_ROOT_SIGNATURE_DESC1 & rootSignatureDesc):
                                                         __GEComputePipelineState(_shader),
                                                         pipelineState(state),
                                                         rootSignature(signature),rootSignatureDesc(rootSignatureDesc){};

GED3D12ComputePipelineState::~GED3D12ComputePipelineState() {
    delete [] rootSignatureDesc.pParameters;
    delete [] rootSignatureDesc.pStaticSamplers;
}

_NAMESPACE_END_