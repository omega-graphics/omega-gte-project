#include "GED3D12.h"
#include "GED3D12CommandQueue.h"
#include "GED3D12Texture.h"
#include "GED3D12RenderTarget.h"
#include "GED3D12Pipeline.h"

#include <atlstr.h>
#include <cassert>


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
    res_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
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

    // void GED3D12Engine::getHardwareAdapter(__in IDXGIFactory4 * dxgi_factory,
    //                                        __out IDXGIAdapter1 **adapter){
    //     ComPtr<IDXGIAdapter1> _out;

    //     ComPtr<IDXGIFactory6> factory6;

    //     BOOL hasDxgiFactory6 = SUCCEEDED(dxgi_factory->QueryInterface(IID_PPV_ARGS(&factory6)));
      
    //     HRESULT hr = S_OK;
    //     UINT adapterIdx = 0;
    //     while(hr != DXGI_ERROR_NOT_FOUND){
            
    //         if(hasDxgiFactory6){
    //             hr = factory6->EnumAdapterByGpuPreference(adapterIdx,DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,IID_PPV_ARGS(&_out));
    //         }
    //         else {
    //             hr = dxgi_factory->EnumAdapters1(adapterIdx,&_out);
    //         }

    //         if(hr == DXGI_ERROR_NOT_FOUND)
    //             break;
            
    //         DXGI_ADAPTER_DESC1 adapterDesc;
    //         _out->GetDesc1(&adapterDesc);

    //         if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE){
    //             /// Skip DXGI Software Warp Adapter.
    //             continue;
    //         }
    //         else {
    //             break;
    //         }
    //         ++adapterIdx;
    //     };

    //     *adapter = _out.Detach();
    // };


    GED3D12Engine::GED3D12Engine(){
        HRESULT hr;

        hr = CreateDXGIFactory2(0,IID_PPV_ARGS(&dxgi_factory));

        if(FAILED(hr)){
            exit(1);
        };

        // ComPtr<IDXGIAdapter1> hardwareAdapter;
        // getHardwareAdapter(dxgi_factory.Get(),&hardwareAdapter);

        D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface));
        debug_interface->EnableDebugLayer();
        debug_interface->SetEnableGPUBasedValidation(true);

        hr = D3D12CreateDevice(NULL,D3D_FEATURE_LEVEL_12_0,IID_PPV_ARGS(&d3d12_device));
        if(FAILED(hr)){
            exit(1);
        };

        DEBUG_STREAM("GED3D12Engine Intialized!");

    };

    SharedHandle<GTEShader> GED3D12Engine::_loadShaderFromDesc(omegasl_shader *shaderDesc) {
        auto shader = new GED3D12Shader();
        shader->internal = *shaderDesc;
        shader->shaderBytecode.pShaderBytecode = shaderDesc->data;
        shader->shaderBytecode.BytecodeLength = shaderDesc->dataSize;
        return SharedHandle<GTEShader>(shader);
    }

    typedef unsigned char D3DByte;

    class GED3D12BufferWriter : public GEBufferWriter {
        GED3D12Buffer * _buffer = nullptr;
        D3DByte *_data_buffer = nullptr;
        size_t currentOffset = 0;
    public:
        void setOutputBuffer(SharedHandle<GEBuffer> &buffer) override {
            currentOffset = 0;
            _buffer = (GED3D12Buffer *)buffer.get();
            CD3DX12_RANGE range(0,0);

            _buffer->buffer->Map(0,&range,(void **)&_data_buffer);
        }
        void structBegin() override {

        }
        void writeFloat(float &v) override {
            memcpy(_data_buffer + currentOffset,&v,sizeof(v));
            currentOffset += sizeof(v);
        }
        void writeFloat2(FVec<2> &v) override {
           DirectX::XMFLOAT2 _v {v[0][0],v[1][0]};
            memcpy(_data_buffer + currentOffset,&_v,sizeof(_v));
            currentOffset += sizeof(_v);
        }
        void writeFloat3(FVec<3> &v) override {
            DirectX::XMFLOAT3 _v {v[0][0],v[1][0],v[2][0]};
            memcpy(_data_buffer + currentOffset,&_v,sizeof(_v));
            currentOffset += sizeof(_v);
        }
        void writeFloat4(FVec<4> &v) override {
            DirectX::XMFLOAT4 _v {v[0][0],v[1][0],v[2][0],v[3][0]};
            memcpy(_data_buffer + currentOffset,&_v,sizeof(_v));
            currentOffset += sizeof(_v);
        }
        void structEnd() override {

        }
        void finish() override {
            _buffer->buffer->Unmap(0,nullptr);
            _buffer = nullptr;
            _data_buffer = nullptr;
            currentOffset = 0;

        }
    };

    SharedHandle<GEBufferWriter> GEBufferWriter::Create() {
        return SharedHandle<GEBufferWriter>(new GED3D12BufferWriter());
    }

    class GED3D12BufferReader : public GEBufferReader {
        GED3D12Buffer * _buffer = nullptr;
        D3DByte *_data_buffer = nullptr;
        size_t currentOffset = 0;
    public:
        void setInputBuffer(SharedHandle<GEBuffer> &buffer) override {
            currentOffset = 0;
            _buffer = (GED3D12Buffer *)buffer.get();
            CD3DX12_RANGE range(0,0);

            _buffer->buffer->Map(0,&range,(void **)&_data_buffer);
        }
        void structBegin() override {

        }
        void getFloat(float &v) override {
            memcpy(&v,_data_buffer + currentOffset,sizeof(v));
            currentOffset += sizeof(v);
        }
        void getFloat2(FVec<2> &v) override {
            DirectX::XMFLOAT2 _v {};
            memcpy(&_v,_data_buffer + currentOffset,sizeof(_v));
            currentOffset += sizeof(_v);
            v[0][0] = _v.x;
            v[1][0] = _v.y;
        }
        void getFloat3(FVec<3> &v) override {
            DirectX::XMFLOAT3 _v {};
            memcpy(&_v,_data_buffer + currentOffset,sizeof(_v));
            currentOffset += sizeof(_v);
            v[0][0] = _v.x;
            v[1][0] = _v.y;
            v[2][0] = _v.z;
        }
        void getFloat4(FVec<4> &v) override {
            DirectX::XMFLOAT4 _v {};
            memcpy(&_v,_data_buffer + currentOffset,sizeof(_v));
            currentOffset += sizeof(_v);
            v[0][0] = _v.x;
            v[1][0] = _v.y;
            v[2][0] = _v.z;
            v[3][0] = _v.w;
        }
        void structEnd() override {

        }
        void finish() override {
            _buffer = nullptr;
            _data_buffer = nullptr;
            currentOffset = 0;
            _buffer->buffer->Unmap(0,nullptr);
        }
    };

    SharedHandle<GEBufferReader> GEBufferReader::Create() {
        return SharedHandle<GEBufferReader>(new GED3D12BufferReader());
    }


    IDXGISwapChain3 *GED3D12Engine::createSwapChainForComposition(DXGI_SWAP_CHAIN_DESC1 *desc,SharedHandle<GECommandQueue> & commandQueue){
        auto *d3d12_queue = (GED3D12CommandQueue *)commandQueue.get();
        IDXGISwapChain1 *swapChain;
        HRESULT hr = dxgi_factory->CreateSwapChainForComposition(d3d12_queue->commandQueue.Get(),desc,nullptr,&swapChain);
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
        return SharedHandle<OmegaGraphicsEngine>(new GED3D12Engine());
    }

    SharedHandle<GEFence> GED3D12Engine::makeFence(){
        ID3D12Fence *f;
        d3d12_device->CreateFence(0,D3D12_FENCE_FLAG_SHARED,IID_PPV_ARGS(&f));
        return std::make_shared<GED3D12Fence>(f);
    };

    SharedHandle<GEHeap> GED3D12Engine::makeHeap(const HeapDescriptor &desc){
        return nullptr;
    };

    SharedHandle<GERenderPipelineState> GED3D12Engine::makeRenderPipelineState(RenderPipelineDescriptor &desc){
        auto & vertexFunc = desc.vertexFunc->internal;
        auto & fragmentFunc = desc.fragmentFunc->internal;

        std::vector<D3D12_INPUT_ELEMENT_DESC> inputs;

        assert(desc.vertexFunc && "Vertex Function is not provided");
        assert(desc.fragmentFunc && "Fragment Function is not provided");

        assert(vertexFunc.type == OMEGASL_SHADER_VERTEX && "Function is not a vertex function");
        assert(fragmentFunc.type == OMEGASL_SHADER_FRAGMENT && "Function is not a fragment function");

        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc {};
        if(vertexFunc.vertexShaderInputDesc.useVertexID){
            auto el = new D3D12_INPUT_ELEMENT_DESC {"SV_VertexID",0,DXGI_FORMAT_R32_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0};
            inputLayoutDesc.NumElements = 1;
            inputLayoutDesc.pInputElementDescs = el;
        }
        else {
            ArrayRef<omegasl_vertex_shader_param_desc> inputDesc{vertexFunc.vertexShaderInputDesc.pParams,
                                                                 vertexFunc.vertexShaderInputDesc.pParams +
                                                                 vertexFunc.vertexShaderInputDesc.nParam};
            for (auto &attr: inputDesc) {
                D3D12_INPUT_ELEMENT_DESC inputEl {};
                switch (attr.type) {
                    case OMEGASL_FLOAT : {
                        inputEl.Format = DXGI_FORMAT_R32_FLOAT;
                        break;
                    }
                    case OMEGASL_FLOAT2 : {
                        inputEl.Format = DXGI_FORMAT_R32G32_FLOAT;
                        break;
                    }
                    case OMEGASL_FLOAT3 : {
                        inputEl.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                        break;
                    }
                    case OMEGASL_FLOAT4 : {
                        inputEl.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                        break;
                    }
                    case OMEGASL_INT : {
                        inputEl.Format = DXGI_FORMAT_R32_SINT;
                        break;
                    }
                    case OMEGASL_INT2 : {
                        inputEl.Format = DXGI_FORMAT_R32G32_SINT;
                        break;
                    }
                    case OMEGASL_INT3 : {
                        inputEl.Format = DXGI_FORMAT_R32G32B32_SINT;
                        break;
                    }
                    case OMEGASL_INT4 : {
                        inputEl.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                        break;
                    }
                };

                inputEl.InputSlot = 0;
                inputEl.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
                inputEl.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                inputs.push_back(inputEl);

            }
            inputLayoutDesc.pInputElementDescs = inputs.data();
            inputLayoutDesc.NumElements = vertexFunc.vertexShaderInputDesc.nParam;

        };


        MessageBoxA(GetForegroundWindow(),"Creating Pipeline State","NOTE",MB_OK);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC d {};
        d.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        d.NodeMask = d3d12_device->GetNodeCount();
        d.InputLayout = inputLayoutDesc;
        
        HRESULT hr;

        omegasl_shader shaders[] = {desc.vertexFunc->internal,desc.fragmentFunc->internal};

        ID3D12RootSignature *signature;
        createRootSignatureFromOmegaSLShaders(2,shaders,&signature);

        GED3D12Shader *vertexShader = (GED3D12Shader *)desc.vertexFunc.get(),
        *fragmentShader = (GED3D12Shader *)desc.fragmentFunc.get();

        d.VS = vertexShader->shaderBytecode;
        d.PS = fragmentShader->shaderBytecode;
        d.pRootSignature = signature;
        d.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        d.NumRenderTargets = 1;
        d.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

        D3D12_CULL_MODE cullMode;
        switch (desc.cullMode) {
            case RasterCullMode::None : {
                cullMode = D3D12_CULL_MODE_NONE;
                break;
            }
            case RasterCullMode::Front : {
                cullMode = D3D12_CULL_MODE_FRONT;
                break;
            }
            case RasterCullMode::Back : {
                cullMode = D3D12_CULL_MODE_BACK;
                break;
            }
        }

        d.RasterizerState.CullMode = cullMode;
        
        D3D12_FILL_MODE fillMode;
        switch (desc.triangleFillMode) {
            case TriangleFillMode::Wireframe : {
                fillMode = D3D12_FILL_MODE_WIREFRAME;
                break;
            }
            case TriangleFillMode::Solid : {
                fillMode = D3D12_FILL_MODE_SOLID;
                break;
            }
        }
        
        d.RasterizerState.FillMode = fillMode;
        d.RasterizerState.ForcedSampleCount = desc.rasterSampleCount;
        d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        d.DepthStencilState.StencilEnable = desc.depthAndStencilDesc.enableStencil;
        d.DepthStencilState.DepthEnable = desc.depthAndStencilDesc.enableDepth;
        d.SampleMask = UINT_MAX;
        d.SampleDesc.Quality = 0;
        d.SampleDesc.Count = 1;

        MessageBoxA(GetForegroundWindow(),"Create Bytecode Funcs","NOTE",MB_OK);
        ID3D12PipelineState *state;
        hr = d3d12_device->CreateGraphicsPipelineState(&d,IID_PPV_ARGS(&state));
        if(FAILED(hr)){
            MessageBoxA(GetForegroundWindow(),"Failed to Create Pipeline State","NOTE",MB_OK);
            exit(1);
        };
        return std::make_shared<GED3D12RenderPipelineState>(desc.vertexFunc,desc.fragmentFunc,state,signature);
    };
    SharedHandle<GEComputePipelineState> GED3D12Engine::makeComputePipelineState(ComputePipelineDescriptor &desc){
        D3D12_COMPUTE_PIPELINE_STATE_DESC d;
        HRESULT hr;
        ID3D12PipelineState *state;

        auto computeShader = (GED3D12Shader *)desc.computeFunc.get();

        d.NodeMask = d3d12_device->GetNodeCount();
        d.CS = computeShader->shaderBytecode;
        omegasl_shader shaders[] = {desc.computeFunc->internal};

        ID3D12RootSignature *signature;

        auto b = createRootSignatureFromOmegaSLShaders(1,shaders,&signature);
        if(b) {
            d.pRootSignature = signature;
        }
        else {

        }

        hr = d3d12_device->CreateComputePipelineState(&d,IID_PPV_ARGS(&state));
        return std::make_shared<GED3D12ComputePipelineState>(desc.computeFunc,state,signature);
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
        DXGI_SWAP_CHAIN_DESC1 swapChaindesc {};
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
         swapChaindesc.SampleDesc.Quality = 0;

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

        

        return std::make_shared<GED3D12NativeRenderTarget>(swapChain,renderTargetHeap,std::move(commandQueue),swapChain->GetCurrentBackBufferIndex(),rtvs.data(),rtvs.size(),desc.hwnd);
    };

    SharedHandle<GETextureRenderTarget> GED3D12Engine::makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc){
        return nullptr;   
    };

    SharedHandle<GECommandQueue> GED3D12Engine::makeCommandQueue(unsigned int maxBufferCount){
        return std::make_shared<GED3D12CommandQueue>(this,maxBufferCount);
    };

    SharedHandle<GETexture> GED3D12Engine::makeTexture(const TextureDescriptor &desc){
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
        }
        else {
            res_states = D3D12_RESOURCE_STATE_COMMON;
        };

         res_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        D3D12_RENDER_TARGET_VIEW_DESC view_desc;
        if(desc.type == GETexture::Texture1D){
            d3d12_desc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,1,desc.mipLevels);
            res_view_desc.Texture1D.MostDetailedMip = -1;
            res_view_desc.Texture1D.MipLevels = desc.mipLevels;
            res_view_desc.Texture1D.ResourceMinLODClamp = 2.f;
        }
        else if(desc.type == GETexture::Texture2D){
            d3d12_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height,1,desc.mipLevels,desc.sampleCount);
            if(desc.sampleCount > 1){
                res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
            else {
                res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            }
            res_view_desc.Texture2D.MipLevels = desc.mipLevels;
            res_view_desc.Texture2D.MostDetailedMip = -1;
            res_view_desc.Texture2D.PlaneSlice = 0;
            res_view_desc.Texture2D.ResourceMinLODClamp = 2.f;

             if(desc.usage & GETexture::RenderTarget){
                 if(desc.sampleCount > 1) {
                     view_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                 }
                 else {
                     view_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                 }
                  view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                  view_desc.Texture2D.PlaneSlice = 0;
                  view_desc.Texture2D.MipSlice = 0;
             }
        }
        else if(desc.type == GETexture::Texture3D){
           d3d12_desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height,desc.depth,desc.mipLevels);
           d3d12_desc.SampleDesc.Count = desc.sampleCount;
           d3d12_desc.DepthOrArraySize = 1;
           res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
           res_view_desc.Texture3D.ResourceMinLODClamp = 2.f;
           res_view_desc.Texture3D.MostDetailedMip = -1;
           res_view_desc.Texture3D.MipLevels = desc.mipLevels;

           if(desc.usage & GETexture::RenderTarget){
                 view_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                 view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                 view_desc.Texture3D.MipSlice = 0;
                 view_desc.Texture3D.FirstWSlice = 0;
                 view_desc.Texture3D.WSize = desc.depth;
            }
        };

        ID3D12Resource *texture;
        CD3DX12_HEAP_PROPERTIES heap_prop;


        if(desc.usage & GETexture::GPUWrite) {
            heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        }
        else if(desc.usage & GETexture::MSResolveDest){
            heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        }
        else {
            heap_prop = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
        }


        hr = d3d12_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,&d3d12_desc,res_states,nullptr,IID_PPV_ARGS(&texture));
        if(FAILED(hr)){

        };

        ID3D12DescriptorHeap *descHeap = nullptr, *rtvDescHeap = nullptr;

        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
        descHeapDesc.NumDescriptors = 1;
        descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        descHeapDesc.NodeMask = d3d12_device->GetNodeCount();

        if(desc.usage & GETexture::GPURead || desc.usage & GETexture::GPUWrite) {
            descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

            hr = d3d12_device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));
            if (FAILED(hr)) {

            };

            d3d12_device->CreateShaderResourceView(texture, &res_view_desc,
                                                   descHeap->GetCPUDescriptorHandleForHeapStart());

        }

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
        D3D12_HEAP_TYPE heap_type;

        switch (desc.usage) {
            case BufferDescriptor::Upload : {
                heap_type = D3D12_HEAP_TYPE_UPLOAD;
                break;
            }
            case BufferDescriptor::Readback : {
                heap_type = D3D12_HEAP_TYPE_READBACK;
                break;
            }
        }
        auto heap_prop = CD3DX12_HEAP_PROPERTIES( heap_type );
        hr = d3d12_device->CreateCommittedResource(
            &heap_prop,D3D12_HEAP_FLAG_NONE,
            &d3d12_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,IID_PPV_ARGS(&buffer));
        if(FAILED(hr)){
            ///
            DEBUG_STREAM("Failed to Create D3D12 Buffer");
            exit(1);
        };

        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc {};
        descHeapDesc.NumDescriptors = 1;
        descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        descHeapDesc.NodeMask = d3d12_device->GetNodeCount();
        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        ID3D12DescriptorHeap *descHeap;
        hr = d3d12_device->CreateDescriptorHeap(&descHeapDesc,IID_PPV_ARGS(&descHeap));
        if(FAILED(hr)){

        };

        if(desc.usage == BufferDescriptor::Upload) {

            D3D12_SHADER_RESOURCE_VIEW_DESC res_view_desc{};
            res_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            res_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            res_view_desc.Format = DXGI_FORMAT_UNKNOWN;
            res_view_desc.Buffer.StructureByteStride = desc.objectStride;
            res_view_desc.Buffer.FirstElement = 0;
            res_view_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            res_view_desc.Buffer.NumElements = desc.len / desc.objectStride;

            d3d12_device->CreateShaderResourceView(buffer, &res_view_desc,
                                                   descHeap->GetCPUDescriptorHandleForHeapStart());
        }
        else {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
            uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            uav_desc.Buffer.NumElements = desc.len/desc.objectStride;
            uav_desc.Buffer.StructureByteStride = desc.objectStride;
            uav_desc.Buffer.CounterOffsetInBytes = 0;
            uav_desc.Buffer.FirstElement = 0;

            d3d12_device->CreateUnorderedAccessView(buffer,nullptr,&uav_desc,descHeap->GetCPUDescriptorHandleForHeapStart());
        }

        return SharedHandle<GEBuffer>(new GED3D12Buffer(buffer,descHeap));
    }

    inline D3D12_TEXTURE_ADDRESS_MODE convertAddressMode(const omegasl_shader_static_sampler_address_mode & addressMode){
        switch (addressMode) {
            case OMEGASL_SHADER_SAMPLER_ADDRESS_MODE_WRAP : {
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            }
            case OMEGASL_SHADER_SAMPLER_ADDRESS_MODE_CLAMPTOEDGE : {
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            }
            case OMEGASL_SHADER_SAMPLER_ADDRESS_MODE_MIRRORWRAP : {
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            }
            default : {
                break;
            }
        }
    }

    inline D3D12_TEXTURE_ADDRESS_MODE convertAddressModeGTE(const SamplerDescriptor::AddressMode & addressMode){
        switch (addressMode) {
            case SamplerDescriptor::AddressMode::Wrap : {
                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            }
            case SamplerDescriptor::AddressMode::ClampToEdge : {
                return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            }
            case SamplerDescriptor::AddressMode::MirrorWrap : {
                return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            }
            default : {
                break;
            }
        }
    }


    bool GED3D12Engine::createRootSignatureFromOmegaSLShaders(unsigned int shaderN, omegasl_shader *shader,
                                                              ID3D12RootSignature **pRootSignature) {
        HRESULT hr;
        ArrayRef<omegasl_shader> shaders {shader,shader + shaderN};

        std::vector<D3D12_ROOT_PARAMETER1> params;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

        for(auto & s : shaders){
            ArrayRef<omegasl_shader_layout_desc> sLayout {s.pLayout,s.pLayout + s.nLayout};
            for(auto & l : sLayout){
                CD3DX12_ROOT_PARAMETER1 parameter1;
                if(l.type == OMEGASL_SHADER_SAMPLER2D_DESC || l.type == OMEGASL_SHADER_SAMPLER3D_DESC){
                    CD3DX12_DESCRIPTOR_RANGE1 desc_table;
                    desc_table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,1,l.gpu_relative_loc);
                    parameter1.InitAsDescriptorTable(1,&desc_table);
                }
                else if(l.type == OMEGASL_SHADER_STATIC_SAMPLER2D_DESC || l.type == OMEGASL_SHADER_STATIC_SAMPLER3D_DESC){
                    D3D12_FILTER filter;

                    switch (l.sampler_desc.filter) {
                        case OMEGASL_SHADER_SAMPLER_LINEAR_FILTER : {
                            filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                            break;
                        }
                        case OMEGASL_SHADER_SAMPLER_POINT_FILTER : {
                            filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
                            break;
                        }
                        case OMEGASL_SHADER_SAMPLER_MAX_ANISOTROPY_FILTER : {
                            filter = D3D12_FILTER_MAXIMUM_ANISOTROPIC;
                            break;
                        }
                        case OMEGASL_SHADER_SAMPLER_MIN_ANISOTROPY_FILTER : {
                            filter = D3D12_FILTER_MINIMUM_ANISOTROPIC;
                            break;
                        }
                    }

                    CD3DX12_STATIC_SAMPLER_DESC desc;
                    desc.Init(l.gpu_relative_loc,filter, convertAddressMode(l.sampler_desc.u_address_mode),
                              convertAddressMode(l.sampler_desc.v_address_mode), convertAddressMode(l.sampler_desc.w_address_mode));
                    staticSamplers.push_back(desc);
                    continue;
                }
                else {
                    parameter1.InitAsShaderResourceView(l.gpu_relative_loc);
                }
                params.push_back(parameter1);
            }
        }

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
        /// @note Always allow input layout regardless of pipeline type.
        desc.Init_1_1(params.size(),params.data(),staticSamplers.size(),staticSamplers.data(),D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ComPtr<ID3DBlob> sigBlob;
        hr = D3DX12SerializeVersionedRootSignature(&desc,D3D_ROOT_SIGNATURE_VERSION_1_1,&sigBlob,nullptr);

        if(FAILED(hr)){
            return false;
        }

        hr = d3d12_device->CreateRootSignature(d3d12_device->GetNodeCount(),sigBlob->GetBufferPointer(),sigBlob->GetBufferSize(),IID_PPV_ARGS(pRootSignature));
        if(FAILED(hr)){
            return false;
        }
        return true;
    };


    SharedHandle<GESamplerState> GED3D12Engine::makeSamplerState(const SamplerDescriptor &desc) {
        D3D12_DESCRIPTOR_HEAP_DESC desc1 {};
        desc1.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc1.NodeMask = d3d12_device->GetNodeCount();
        desc1.NumDescriptors = 1;
        desc1.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        D3D12_FILTER filter;

        switch (desc.filter) {
            case SamplerDescriptor::Filter::Linear : {
                filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                break;
            }
            case SamplerDescriptor::Filter::Point : {
                filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
                break;
            }
            case SamplerDescriptor::Filter::MaxAnisotropic : {
                filter = D3D12_FILTER_MAXIMUM_ANISOTROPIC;
                break;
            }
            case SamplerDescriptor::Filter::MinAnisotropic : {
                filter = D3D12_FILTER_MINIMUM_ANISOTROPIC;
                break;
            }
        }

        ID3D12DescriptorHeap *descHeap;
        d3d12_device->CreateDescriptorHeap(&desc1,IID_PPV_ARGS(&descHeap));
        D3D12_SAMPLER_DESC samplerDesc {};
        samplerDesc.AddressU = convertAddressModeGTE(desc.uAddressMode);
        samplerDesc.AddressV = convertAddressModeGTE(desc.vAddressMode);
        samplerDesc.AddressW = convertAddressModeGTE(desc.wAddressMode);
        samplerDesc.Filter = filter;
        samplerDesc.MaxAnisotropy = desc.maxAnisotropy;
        
        d3d12_device->CreateSampler(&samplerDesc,descHeap->GetCPUDescriptorHandleForHeapStart());

        return SharedHandle<GESamplerState>(new GED3D12SamplerState(descHeap,samplerDesc));
    }

_NAMESPACE_END_