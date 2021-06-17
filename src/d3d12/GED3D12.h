#include "omegaGTE/GE.h"

#include "d3dx12.h"
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3d12shader.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <pix.h>
#include <iostream>

#include <wrl.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"runtimeobject.lib")
#pragma comment(lib,"d3dcompiler.lib")

#ifndef OMEGAGTE_GED3D12_H
#define OMEGAGTE_GED3D12_H

_NAMESPACE_BEGIN_
    using Microsoft::WRL::ComPtr;

    class GED3D12Buffer : public GEBuffer {
    public:

        ComPtr<ID3D12Resource> buffer;
        ComPtr<ID3D12DescriptorHeap> bufferDescHeap;

        size_t size(){
            return buffer->GetDesc().Width;
        };
        void * data(){
            void *ptr;
            CD3DX12_RANGE readRange(0,0);
            buffer->Map(0,&readRange,&ptr);
            return ptr;
        };
        void removePtrRef(){
             buffer->Unmap(0,nullptr);
        };
        GED3D12Buffer(ID3D12Resource *buffer,ID3D12DescriptorHeap *bufferDescHeap):buffer(buffer),bufferDescHeap(bufferDescHeap){
            
        };
    };

    class GED3D12Fence : public GEFence {
    public:
        ComPtr<ID3D12Fence> fence;
        GED3D12Fence(ID3D12Fence *fence):fence(fence){};
    };

    class GED3D12Engine;

    class GED3D12Heap : public GEHeap {
        GED3D12Engine *engine;
        ComPtr<ID3D12Heap> heap;
        size_t currentOffset;
    public:
        GED3D12Heap(GED3D12Engine *engine,ID3D12Heap * heap):engine(engine),heap(heap),currentOffset(0){};
        size_t currentSize(){
            return heap->GetDesc().SizeInBytes;
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc);
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc);
    };

    class GED3D12Engine : public OmegaGraphicsEngine {
    public:
        GED3D12Engine();
        ComPtr<IDXGIFactory4> dxgi_factory;
        ComPtr<ID3D12Device8> d3d12_device;
        // ComPtr<ID3D12DescriptorHeap> descriptorHeapForRes;
        static SharedHandle<OmegaGraphicsEngine> Create();
        void getHardwareAdapter(__in IDXGIFactory4 * dxgi_factory,__out IDXGIAdapter1 **adapter);
        SharedHandle<GEFunctionLibrary> loadShaderLibrary(FS::Path path);
        SharedHandle<GEFunctionLibrary> loadStdShaderLibrary();
        SharedHandle<GEFence> makeFence();
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc);
        SharedHandle<GEHeap> makeHeap(const HeapDescriptor &desc);
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount);
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc);
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor &desc);
        SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor &desc);
        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc);
        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc);
        IDXGISwapChain3 *createSwapChainForComposition(DXGI_SWAP_CHAIN_DESC1 *desc,SharedHandle<GECommandQueue> & commandQueue);
        IDXGISwapChain3 *createSwapChainFromHWND(HWND hwnd,DXGI_SWAP_CHAIN_DESC1 *desc,SharedHandle<GECommandQueue> & commandQueue);
    };
_NAMESPACE_END_
#endif




