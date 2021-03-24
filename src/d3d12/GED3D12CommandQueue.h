#include "omegaGTE/GECommandQueue.h"
#include "GED3D12.h"

#ifndef OMEGAGRAPHICSENGINE_GED3D12COMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_GED3D12COMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GED3D12CommandQueue;

    class GED3D12CommandBuffer : public GECommandBuffer {
        ComPtr<ID3D12GraphicsCommandList4> commandList;
        GED3D12CommandQueue *parentQueue;
    public:
        GED3D12CommandBuffer(ID3D12GraphicsCommandList4 *commandList,GED3D12CommandQueue *parentQueue);
    };

    class GED3D12CommandQueue : public GECommandQueue {
        GED3D12Engine *engine;
        ComPtr<ID3D12CommandQueue> commandQueue;
        unsigned currentCount;
        friend class GED3D12Engine;
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GED3D12CommandQueue(GED3D12Engine *engine,unsigned size);
    };
_NAMESPACE_END_
#endif
