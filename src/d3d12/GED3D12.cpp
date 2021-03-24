#include "GED3D12.h"
#include "GED3D12CommandQueue.h"
#include "GED3D12Texture.h"
_NAMESPACE_BEGIN_

    class GED3D12Buffer : public GEBuffer {
        ComPtr<ID3D12Resource> buffer;
    public:
        GED3D12Buffer(ID3D12Resource *buffer):buffer(buffer){};
    };

    GED3D12Engine::GED3D12Engine(){
        HRESULT hr;

        hr = D3D12CreateDevice(NULL,D3D_FEATURE_LEVEL_12_1,IID_PPV_ARGS(d3d12_device.GetAddressOf()));
        if(FAILED(hr)){
            exit(1);
        };
        hr = d3d12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(bufferAllocator.GetAddressOf()));
        if(FAILED(hr)){
            exit(1);
        };

        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = d3d12_device->GetNodeCount();
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        hr = d3d12_device->CreateDescriptorHeap(&desc,IID_PPV_ARGS(&descriptorHeapForRes));
        if(FAILED(hr)){

        };

    };

    SharedHandle<GENativeRenderTarget> GED3D12Engine::makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc){
        HRESULT hr;
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heap_desc.NodeMask = d3d12_device->GetNodeCount();
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        ID3D12DescriptorHeap *renderTargetHeap;
        hr = d3d12_device->CreateDescriptorHeap(&heap_desc,IID_PPV_ARGS(&renderTargetHeap));
        if(FAILED(hr)){

        };

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Flags = 0;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.Width = 0;
        swapChainDesc.Height = 0;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        IDXGISwapChain1 *swapChain;

        auto commandQueue = std::make_shared<GED3D12CommandQueue>(this,64);

        hr = dxgi_factory->CreateSwapChainForHwnd(commandQueue->commandQueue.Get(),desc.hwnd,&swapChainDesc,NULL,NULL,&swapChain);
        if(FAILED(hr)){

        };
        IDXGISwapChain3 * swapChain_;
        hr = swapChain->QueryInterface(&swapChain_);
        if(FAILED(hr)){

        };
        
    };

    SharedHandle<GECommandQueue> GED3D12Engine::makeCommandQueue(unsigned int maxBufferCount){
        return std::make_shared<GED3D12CommandQueue>(this,maxBufferCount);
    };

    SharedHandle<GETexture> GED3D12Engine::makeTexture(const TextureDescriptor &desc){\
        HRESULT hr;
        D3D12_RESOURCE_DESC d3d12_desc;
        D3D12_RESOURCE_STATES res_states;

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
        }
        else if(desc.type == GETexture::Texture3D){
           d3d12_desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UNORM,desc.width,desc.height,desc.depth);
        };
        ID3D12Resource *texture;
        auto heap_prop = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );
        hr = d3d12_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,&d3d12_desc,res_states,nullptr,IID_PPV_ARGS(&texture));
        if(FAILED(hr)){

        };
        return std::make_shared<GED3D12Texture>(texture);
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