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

    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    descHeapDesc.NumDescriptors = 1;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descHeapDesc.NodeMask = engine->d3d12_device->GetNodeCount();
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    ID3D12DescriptorHeap *descHeap;
    hr = engine->d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&descHeap));
    if(FAILED(hr)){

    };

    D3D12_SHADER_RESOURCE_VIEW_DESC res_view_desc;
    res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    res_view_desc.Format = DXGI_FORMAT_UNKNOWN;
    res_view_desc.Buffer.StructureByteStride = desc.objectStride;
    res_view_desc.Buffer.FirstElement = 0;
    res_view_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    res_view_desc.Buffer.NumElements = desc.len/desc.objectStride;

    engine->d3d12_device->CreateShaderResourceView(buffer,&res_view_desc,descHeap->GetCPUDescriptorHandleForHeapStart());


    auto alloc_info = engine->d3d12_device->GetResourceAllocationInfo(engine->d3d12_device->GetNodeCount(),1,&d3d12_desc);
    currentOffset += alloc_info.SizeInBytes;
    return std::make_shared<GED3D12Buffer>(buffer,descHeap);
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

        res_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        

    D3D12_RENDER_TARGET_VIEW_DESC view_desc;

    view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    if(desc.type == GETexture::Texture2D){
        d3d12_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height);
        res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            if(desc.usage & GETexture::RenderTarget){
                view_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                
            }
    }
    else if(desc.type == GETexture::Texture3D){
        d3d12_desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height,desc.depth);
        res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        if(desc.usage & GETexture::RenderTarget){
                view_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
        }
    };

    ID3D12Resource *texture;
    hr = engine->d3d12_device->CreatePlacedResource(heap.Get(),currentOffset,&d3d12_desc,res_states,nullptr,IID_PPV_ARGS(&texture));
    auto info = engine->d3d12_device->GetResourceAllocationInfo(engine->d3d12_device->GetNodeCount(),1,&d3d12_desc);
    currentOffset += info.SizeInBytes;
    if(FAILED(hr)){

    };
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    descHeapDesc.NumDescriptors = 1;
    descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descHeapDesc.NodeMask = engine->d3d12_device->GetNodeCount();
    descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    ID3D12DescriptorHeap *descHeap, *rtvDescHeap = nullptr;
    hr = engine->d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&descHeap));
    if(FAILED(hr)){

    };

    engine->d3d12_device->CreateShaderResourceView(texture,&res_view_desc,descHeap->GetCPUDescriptorHandleForHeapStart());

    if(desc.usage & GETexture::RenderTarget){
        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    
        hr = engine->d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&rtvDescHeap));
        if(FAILED(hr)){

        };

        engine->d3d12_device->CreateRenderTargetView(texture,&view_desc,rtvDescHeap->GetCPUDescriptorHandleForHeapStart());
    };

    return std::make_shared<GED3D12Texture>(texture,descHeap,rtvDescHeap);

};

    void GED3D12Engine::getHardwareAdapter(__in IDXGIFactory4 * dxgi_factory,
                                           __out IDXGIAdapter1 **adapter){
        ComPtr<IDXGIAdapter1> _out;

        ComPtr<IDXGIFactory6> factory6;

        BOOL hasDxgiFactory6 = SUCCEEDED(dxgi_factory->QueryInterface(IID_PPV_ARGS(&factory6)));
      
        HRESULT hr = S_OK;
        UINT adapterIdx = 0;
        while(hr != DXGI_ERROR_NOT_FOUND){
            
            if(hasDxgiFactory6){
                hr = factory6->EnumAdapterByGpuPreference(adapterIdx,DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,IID_PPV_ARGS(&_out));
            }
            else {
                hr = dxgi_factory->EnumAdapters1(adapterIdx,&_out);
            }

            if(hr == DXGI_ERROR_NOT_FOUND)
                break;
            
            DXGI_ADAPTER_DESC1 adapterDesc;
            _out->GetDesc1(&adapterDesc);

            if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE){
                /// Skip DXGI Software Warp Adapter.
                continue;
            }
            else {
                break;
            }
            ++adapterIdx;
        };

        *adapter = _out.Detach();
    };


    GED3D12Engine::GED3D12Engine(){
        HRESULT hr;

        hr = CreateDXGIFactory2(0,IID_PPV_ARGS(&dxgi_factory));

        if(FAILED(hr)){
            exit(1);
        };

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        getHardwareAdapter(dxgi_factory.Get(),&hardwareAdapter);

        hr = D3D12CreateDevice(hardwareAdapter.Get(),D3D_FEATURE_LEVEL_12_1,IID_PPV_ARGS(d3d12_device.GetAddressOf()));
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

    IDXGISwapChain3 *GED3D12Engine::createSwapChainForComposition(DXGI_SWAP_CHAIN_DESC1 *desc,SharedHandle<GECommandQueue> & commandQueue){
        GED3D12CommandQueue *d3d12_queue = (GED3D12CommandQueue *)commandQueue.get();
        IDXGISwapChain1 *swapChain;
        HRESULT hr = dxgi_factory->CreateSwapChainForComposition(d3d12_queue->commandQueue.Get(),desc,NULL,&swapChain);
        if(FAILED(hr)){
            exit(1);
        };
        IDXGISwapChain3 *lswapChain;
        hr = swapChain->QueryInterface(&lswapChain);
        if(FAILED(hr)){
            exit(1);
        };
        return lswapChain;
    }

    IDXGISwapChain3 *GED3D12Engine::createSwapChainFromHWND(HWND hwnd,DXGI_SWAP_CHAIN_DESC1 *desc,SharedHandle<GECommandQueue> & commandQueue){
        GED3D12CommandQueue *d3d12_queue = (GED3D12CommandQueue *)commandQueue.get();
        IDXGISwapChain1 *swapChain;
        HRESULT hr = dxgi_factory->CreateSwapChainForHwnd(d3d12_queue->commandQueue.Get(),hwnd,desc,NULL,NULL,&swapChain);
        if(FAILED(hr)){
             MessageBoxA(GetForegroundWindow(),"Failed to Create SwapChain.","NOTE",MB_OK);
            exit(1);
        };
        IDXGISwapChain3 *lswapChain;
        hr = swapChain->QueryInterface(&lswapChain);
        if(FAILED(hr)){
            MessageBoxA(GetForegroundWindow(),"Failed to Query SwapChain.","NOTE",MB_OK);
            exit(1);
        };
        return lswapChain;
    }

    SharedHandle<OmegaGraphicsEngine> GED3D12Engine::Create(){
        return std::make_shared<GED3D12Engine>();
    }

    SharedHandle<GEFence> GED3D12Engine::makeFence(){
        ID3D12Fence *f;
        d3d12_device->CreateFence(0,D3D12_FENCE_FLAG_SHARED,IID_PPV_ARGS(&f));
        return std::make_shared<GED3D12Fence>(f);
    };

    SharedHandle<GEHeap> GED3D12Engine::makeHeap(const HeapDescriptor &desc){
        return nullptr;
    };

    SharedHandle<GERenderPipelineState> GED3D12Engine::makeRenderPipelineState(const RenderPipelineDescriptor &desc){
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputs;
        for(auto & attr : desc.vertexInputAttributes){
            D3D12_INPUT_ELEMENT_DESC inputEl;
            switch (attr.type) {
                case InputAttributeDesc::FLOAT : {
                    inputEl.Format = DXGI_FORMAT_R32_FLOAT;
                    break;
                }
                case InputAttributeDesc::FLOAT2 : {
                    inputEl.Format = DXGI_FORMAT_R32G32_FLOAT;
                    break;
                }
                case InputAttributeDesc::FLOAT3 : {
                    inputEl.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    break;
                }
                case InputAttributeDesc::FLOAT4 : {
                    inputEl.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    break;
                }
                case InputAttributeDesc::INT : {
                    inputEl.Format = DXGI_FORMAT_R32_SINT;
                    break;
                }
                case InputAttributeDesc::INT2 : {
                    inputEl.Format = DXGI_FORMAT_R32G32_SINT;
                    break;
                }
                case InputAttributeDesc::INT3 : {
                    inputEl.Format = DXGI_FORMAT_R32G32B32_SINT;
                    break;
                }
                case InputAttributeDesc::INT4 : {
                    inputEl.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                    break;
                }
            };
            
            inputEl.InputSlot = 0;
            inputEl.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            inputEl.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            inputs.push_back(inputEl);
            
        };

        inputLayoutDesc.pInputElementDescs = inputs.data();
        inputLayoutDesc.NumElements = inputs.size();


        D3D12_GRAPHICS_PIPELINE_STATE_DESC d;
        d.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        d.NodeMask = d3d12_device->GetNodeCount();
        
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
        d.NodeMask = d3d12_device->GetNodeCount();
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
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heap_desc.NodeMask = d3d12_device->GetNodeCount();
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heap_desc.NumDescriptors = 2;
        ID3D12DescriptorHeap *renderTargetHeap;
        hr = d3d12_device->CreateDescriptorHeap(&heap_desc,IID_PPV_ARGS(&renderTargetHeap));
        if(FAILED(hr)){
             MessageBoxA(GetForegroundWindow(),"Failed to Create Descriptor Heap","NOTE",MB_OK);
            exit(1);
        };

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle (renderTargetHeap->GetCPUDescriptorHandleForHeapStart());
        
        RECT rc;
        GetClientRect(desc.hwnd,&rc);
        DXGI_SWAP_CHAIN_DESC1 swapChaindesc = {};
        swapChaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        // swapChaindesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        swapChaindesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChaindesc.BufferCount = 2;
        swapChaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        // swapChaindesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        swapChaindesc.Height = rc.bottom - rc.top;
        swapChaindesc.Width = rc.right - rc.left;
        // swapChaindesc.Scaling = DXGI_SCALING_NONE;
        // swapChaindesc.Stereo = TRUE;
        swapChaindesc.SampleDesc.Count = 1;
        // swapChaindesc.SampleDesc.Quality = 0;

        auto commandQueue = makeCommandQueue(64);

        auto swapChain = createSwapChainFromHWND(desc.hwnd,&swapChaindesc,commandQueue);
       

        std::vector<ID3D12Resource *> rtvs;

        for(unsigned i = 0;i < 2;i++){
            rtvs.resize(i + 1);
            hr = swapChain->GetBuffer(i,IID_PPV_ARGS(&rtvs[i]));
            if(FAILED(hr)){
                exit(1);
            };
            d3d12_device->CreateRenderTargetView(rtvs[i],nullptr,rtv_cpu_handle);
            rtv_cpu_handle.Offset(1,rtv_desc_size);
        };

        

        return std::make_shared<GED3D12NativeRenderTarget>(swapChain,renderTargetHeap,std::move(commandQueue),swapChain->GetCurrentBackBufferIndex(),rtvs.data(),rtvs.size());
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

        D3D12_RENDER_TARGET_VIEW_DESC view_desc;

        if(desc.type == GETexture::Texture2D){
            d3d12_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height);
            res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
             if(desc.usage & GETexture::RenderTarget){
                 view_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                  view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
             }
        }
        else if(desc.type == GETexture::Texture3D){
           d3d12_desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height,desc.depth);
           res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
           if(desc.usage & GETexture::RenderTarget){
                 view_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                 view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            }
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
        ID3D12DescriptorHeap *descHeap, *rtvDescHeap = nullptr;
        hr = d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&descHeap));
        if(FAILED(hr)){

        };

        d3d12_device->CreateShaderResourceView(texture,&res_view_desc,descHeap->GetCPUDescriptorHandleForHeapStart());

        if(desc.usage & GETexture::RenderTarget){
            descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        
            hr = d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&rtvDescHeap));
            if(FAILED(hr)){

            };

            d3d12_device->CreateRenderTargetView(texture,&view_desc,rtvDescHeap->GetCPUDescriptorHandleForHeapStart());
        };

        return std::make_shared<GED3D12Texture>(texture,descHeap,rtvDescHeap);
    };

    SharedHandle<GEBuffer> GED3D12Engine::makeBuffer(const BufferDescriptor &desc){
        HRESULT hr;
        D3D12_RESOURCE_DESC d3d12_desc = CD3DX12_RESOURCE_DESC::Buffer(desc.len);
        ID3D12Resource *buffer;
        auto heap_prop = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
        hr = d3d12_device->CreateCommittedResource(
            &heap_prop,D3D12_HEAP_FLAG_NONE,
            &d3d12_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ | D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,IID_PPV_ARGS(&buffer));
        if(FAILED(hr)){
            ///
            DEBUG_STREAM("Failed to Create D3D12 Buffer");
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

        D3D12_SHADER_RESOURCE_VIEW_DESC res_view_desc;
        res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        res_view_desc.Format = DXGI_FORMAT_UNKNOWN;
        res_view_desc.Buffer.StructureByteStride = desc.objectStride;
        res_view_desc.Buffer.FirstElement = 0;
        res_view_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        res_view_desc.Buffer.NumElements = desc.len/desc.objectStride;

        d3d12_device->CreateShaderResourceView(buffer,&res_view_desc,descHeap->GetCPUDescriptorHandleForHeapStart());

        return std::make_shared<GED3D12Buffer>(buffer,descHeap);
    };

    SharedHandle<GEFunctionLibrary> GED3D12Engine::loadShaderLibrary(FS::Path path){
        return nullptr;
    };
    SharedHandle<GEFunctionLibrary> GED3D12Engine::loadStdShaderLibrary(){
        return nullptr;
    };
_NAMESPACE_END_