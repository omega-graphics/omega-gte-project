#import "GEMetal.h"
#import "GEMetalCommandQueue.h"
#import "GEMetalTexture.h"

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

    class GEMetalBuffer : public GEBuffer {
        id<MTLBuffer> metalBuffer;
    public:
    };
    
    class GEMetalFence : public GEFence {
        id<MTLFence> metalFence;
    public:
        GEMetalFence(id<MTLFence> fence);
    };

    class GEMetalEngine : public OmegaGraphicsEngine {
        id<MTLDevice> metalDevice;
    public:
        GEMetalEngine():metalDevice(MTLCreateSystemDefaultDevice()){
            
        };
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount){
            id<MTLCommandQueue> commandQueue = [metalDevice newCommandQueueWithMaxCommandBufferCount:maxBufferCount+1];
            return std::make_shared<GEMetalCommandQueue>(commandQueue,maxBufferCount);
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc){
            
        };
        SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor &desc){};
        SharedHandle<GEFence> makeFence(){
            return std::make_shared<GEMetalFence>([metalDevice newFence]);
        };
        SharedHandle<GEHeap> makeHeap(const HeapDescriptor &desc){
            MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc]init];
            
        };
        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc){};
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor &desc){};
        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc){};
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc){
            MTLTextureDescriptor *mtlDesc = [[MTLTextureDescriptor alloc] init];
            id<MTLTexture> texture = [metalDevice newTextureWithDescriptor:mtlDesc];
            return std::make_shared<GEMetalTexture>(texture,desc);
        };
    };


    SharedHandle<OmegaGraphicsEngine> CreateMetalEngine(){
        return std::make_shared<GEMetalEngine>();
    };
_NAMESPACE_END_