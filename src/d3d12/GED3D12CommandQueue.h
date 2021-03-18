#include "omegaGE/GECommandQueue.h"
#include "GED3D12.h"

namespace OmegaGE {
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
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GED3D12CommandQueue(GED3D12Engine *engine,unsigned size);
    };
}