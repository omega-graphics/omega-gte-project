#import "GEMetal.h"
#import "GEMetalCommandQueue.h"

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

    class GEMetalEngine : public OmegaGraphicsEngine {
        id<MTLDevice> metalDevice;
    public:
        GEMetalEngine():metalDevice(MTLCreateSystemDefaultDevice()){
            
        };
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount){
            return nullptr;
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc){};
        SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor &desc){};
        SharedHandle<GEFence> makeFence(){};
        SharedHandle<GEHeap> makeHeap(const HeapDescriptor &desc){};
        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc){};
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor &desc){};
        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc){};
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc){};
    };


    SharedHandle<OmegaGraphicsEngine> CreateMetalEngine(){
        return std::make_shared<GEMetalEngine>();
    };
_NAMESPACE_END_