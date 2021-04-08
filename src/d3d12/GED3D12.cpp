#include "GED3D12.h"
#include "GED3D12CommandQueue.h"
#include "GED3D12Texture.h"
#include "GED3D12RenderTarget.h"
#include "GED3D12Pipeline.h"


_NAMESPACE_BEGIN_

SharedHandle<GEBuffer> GED3D12Heap::makeBuffer(const BufferDescriptor &desc){
            HRESULT hr;
            D3D12_RESOURCE_DESC d3d12_desc = CD3DX12_RESOURCE_DESC::Buffer(desc.len);
            ID3D12Resource *buffer;
            hr = engine->d3d12_device->CreatePlacedResource(heap.Get(),currentOffset,&d3d12_desc,D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_GENERIC_READ,NULL,IID_PPV_ARGS(&buffer));
            if(FAILED(hr)){
                exit(1);
            };
            auto alloc_info = engine->d3d12_device->GetResourceAllocationInfo(engine->d3d12_device->GetNodeCount(),1,&d3d12_desc);
            currentOffset += alloc_info.SizeInBytes;
            return std::make_shared<GED3D12Buffer>(buffer);
};

SharedHandle<GETexture> GED3D12Heap::makeTexture(const TextureDescriptor &desc){
            HRESULT hr;
            D3D12_RESOURCE_DESC d3d12_desc;
            D3D12_RESOURCE_STATES res_states;

            D3D12_SHADER_RESOURCE_VIEW_DESC res_view_desc;

            if(desc.usage & GETexture::RenderTarget){
                res_states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
            }
            else if(desc.usage & GETexture::GPURead){
                res_states |= D3D12_RESOURCE_STATE_GENERIC_READ;
            }   
            else if(desc.usage & GETexture::GPUWrite){
                res_states |= D3D12_RESOURCE_STATE_COPY_DEST;
            };

            if(desc.type == GETexture::Texture2D){
                d3d12_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height);
                res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            }
            else if(desc.type == GETexture::Texture3D){
                d3d12_desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height,desc.depth);
                res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            };
            ID3D12Resource *texture;
            hr = engine->d3d12_device->CreatePlacedResource(heap.Get(),currentOffset,&d3d12_desc,D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_GENERIC_READ,NULL,IID_PPV_ARGS(&texture));
            if(FAILED(hr)){
                exit(1);
            };
            auto alloc_info = engine->d3d12_device->GetResourceAllocationInfo(engine->d3d12_device->GetNodeCount(),1,&d3d12_desc);
            currentOffset += alloc_info.SizeInBytes;
            D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
            descHeapDesc.NumDescriptors = 1;
            descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            descHeapDesc.NodeMask = engine->d3d12_device->GetNodeCount();
            descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            ID3D12DescriptorHeap *descHeap;
            hr = engine->d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&descHeap));
            if(FAILED(hr)){

            };

            engine->d3d12_device->CreateShaderResourceView(texture,&res_view_desc,descHeap->GetCPUDescriptorHandleForHeapStart());

            return std::make_shared<GED3D12Texture>(texture,descHeap);
        };


    GED3D12Engine::GED3D12Engine(){
        HRESULT hr;

        hr = D3D12CreateDevice(NULL,D3D_FEATURE_LEVEL_12_1,IID_PPV_ARGS(d3d12_device.GetAddressOf()));
        if(FAILED(hr)){
            exit(1);
        };

        // D3D12_DESCRIPTOR_HEAP_DESC desc;
        // desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        // desc.NodeMask = d3d12_device->GetNodeCount();
        // desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        // hr = d3d12_device->CreateDescriptorHeap(&desc,IID_PPV_ARGS(&descriptorHeapForRes));
        // if(FAILED(hr)){
        //     exit(1);
        // };

    };

    SharedHandle<OmegaGraphicsEngine> GED3D12Engine::Create(){
        return std::make_shared<GED3D12Engine>();
    };

    SharedHandle<GEFence> GED3D12Engine::makeFence(){
        ID3D12Fence *f;
        d3d12_device->CreateFence(0,D3D12_FENCE_FLAG_SHARED,IID_PPV_ARGS(&f));
        return std::make_shared<GED3D12Fence>(f);
    };

    SharedHandle<GEHeap> GED3D12Engine::makeHeap(const HeapDescriptor &desc){
        return nullptr;
    };

    SharedHandle<GERenderPipelineState> GED3D12Engine::makeRenderPipelineState(const RenderPipelineDescriptor &desc){
        D3D12_GRAPHICS_PIPELINE_STATE_DESC d;
        HRESULT hr;
        GED3D12Function *vertexFunc = (GED3D12Function *)desc.vertexFunc.get();
        GED3D12Function *fragmentFunc = (GED3D12Function *)desc.fragmentFunc.get();
        d.VS = CD3DX12_SHADER_BYTECODE(vertexFunc->funcData.Get());
        d.PS = CD3DX12_SHADER_BYTECODE(fragmentFunc->funcData.Get());
        ID3D12PipelineState *state;
        hr = d3d12_device->CreateGraphicsPipelineState(&d,IID_PPV_ARGS(&state));
        return std::make_shared<GED3D12RenderPipelineState>(state);
    };
    SharedHandle<GEComputePipelineState> GED3D12Engine::makeComputePipelineState(const ComputePipelineDescriptor &desc){
        D3D12_COMPUTE_PIPELINE_STATE_DESC d;
        HRESULT hr;
        ID3D12PipelineState *state;
        GED3D12Function *computeFunc = (GED3D12Function *)desc.computeFunc.get();
        d.CS = CD3DX12_SHADER_BYTECODE(computeFunc->funcData.Get());
        hr = d3d12_device->CreateComputePipelineState(&d,IID_PPV_ARGS(&state));
        return std::make_shared<GED3D12ComputePipelineState>(state);
    };

    SharedHandle<GENativeRenderTarget> GED3D12Engine::makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc){
        HRESULT hr;

        /// Swap Chain must have 2 Frames
        auto rtv_desc_size = d3d12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heap_desc.NodeMask = d3d12_device->GetNodeCount();
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heap_desc.NumDescriptors = 2;
        ID3D12DescriptorHeap *renderTargetHeap;
        hr = d3d12_device->CreateDescriptorHeap(&heap_desc,IID_PPV_ARGS(&renderTargetHeap));
        if(FAILED(hr)){

        };

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle (renderTargetHeap->GetCPUDescriptorHandleForHeapStart());
        

        ID3D12Resource *rtv_1;
        d3d12_device->CreateRenderTargetView(rtv_1,nullptr,rtv_cpu_handle);
        rtv_cpu_handle.Offset(1,rtv_desc_size);
        ID3D12Resource *rtv_2;
        d3d12_device->CreateRenderTargetView(rtv_2,nullptr,rtv_cpu_handle);
        rtv_cpu_handle.Offset(1,rtv_desc_size);

        std::initializer_list<ID3D12Resource *> rtvs = {rtv_1,rtv_2};

        auto commandQueue = std::make_shared<GED3D12CommandQueue>(this,64);

        return std::make_shared<GED3D12NativeRenderTarget>(desc.swapChain,renderTargetHeap,commandQueue,desc.swapChain->GetCurrentBackBufferIndex(),rtvs.begin(),rtvs.size());
    };

    SharedHandle<GETextureRenderTarget> GED3D12Engine::makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc){
        return nullptr;   
    };

    SharedHandle<GECommandQueue> GED3D12Engine::makeCommandQueue(unsigned int maxBufferCount){
        return std::make_shared<GED3D12CommandQueue>(this,maxBufferCount);
    };

    SharedHandle<GETexture> GED3D12Engine::makeTexture(const TextureDescriptor &desc){\
        HRESULT hr;
        D3D12_RESOURCE_DESC d3d12_desc;
        D3D12_RESOURCE_STATES res_states;

        D3D12_SHADER_RESOURCE_VIEW_DESC res_view_desc;

        if(desc.usage & GETexture::RenderTarget){
            res_states |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        }
        else if(desc.usage & GETexture::GPURead){
            res_states |= D3D12_RESOURCE_STATE_GENERIC_READ;
        }   
        else if(desc.usage & GETexture::GPUWrite){
            res_states |= D3D12_RESOURCE_STATE_COPY_DEST;
        };

         res_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        if(desc.type == GETexture::Texture2D){
            d3d12_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height);
            res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            // res_view_desc.Texture2D.MipLevels = 1;
        }
        else if(desc.type == GETexture::Texture3D){
           d3d12_desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height,desc.depth);
           res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        };
        ID3D12Resource *texture;
        auto heap_prop = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
        hr = d3d12_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,&d3d12_desc,res_states,nullptr,IID_PPV_ARGS(&texture));
        if(FAILED(hr)){

        };
        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
        descHeapDesc.NumDescriptors = 1;
        descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        descHeapDesc.NodeMask = d3d12_device->GetNodeCount();
        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        ID3D12DescriptorHeap *descHeap;
        hr = d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&descHeap));
        if(FAILED(hr)){

        };

        d3d12_device->CreateShaderResourceView(texture,&res_view_desc,descHeap->GetCPUDescriptorHandleForHeapStart());

        return std::make_shared<GED3D12Texture>(texture,descHeap);
    };

    SharedHandle<GEBuffer> GED3D12Engine::makeBuffer(const BufferDescriptor &desc){
        HRESULT hr;
        D3D12_RESOURCE_DESC d3d12_desc = CD3DX12_RESOURCE_DESC::Buffer(desc.len);
        ID3D12Resource *buffer;
        auto heap_prop = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
        hr = d3d12_device->CreateCommittedResource(&heap_prop,D3D12_HEAP_FLAG_NONE,&d3d12_desc,D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_COPY_DEST,nullptr,IID_PPV_ARGS(&buffer));
        if(FAILED(hr)){

        };
        return std::make_shared<GED3D12Buffer>(buffer);
    };
_NAMESPACE_END_