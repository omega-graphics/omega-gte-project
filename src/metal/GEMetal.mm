#import "GEMetal.h"
#import "GEMetalCommandQueue.h"
#import "GEMetalTexture.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

    class GEMetalBuffer : public GEBuffer {
        id<MTLBuffer> metalBuffer;
    public:
        GEMetalBuffer(id<MTLBuffer> buffer):GEBuffer(),metalBuffer(buffer){};
    };
    
    class GEMetalFence : public GEFence {
        id<MTLFence> metalFence;
    public:
        GEMetalFence(id<MTLFence> fence):GEFence(),metalFence(fence){};
    };

    class GEMetalEngine : public OmegaGraphicsEngine {
        id<MTLDevice> metalDevice;
    public:
        GEMetalEngine():metalDevice(MTLCreateSystemDefaultDevice()){
            DEBUG_STREAM("GEMetalEngine Successfully Created");
        };
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount){
            id<MTLCommandQueue> commandQueue = [metalDevice newCommandQueueWithMaxCommandBufferCount:maxBufferCount+1];
            return std::make_shared<GEMetalCommandQueue>(commandQueue,maxBufferCount);
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc){
            id<MTLBuffer> buffer = [metalDevice newBufferWithLength:desc.len options:MTLResourceStorageModeShared];
            return std::make_shared<GEMetalBuffer>(buffer);
        };
        SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor &desc){
            GEMetalFunction *computeFunc = (GEMetalFunction *)desc.computeFunc.get();
            NSError *error;
            id<MTLComputePipelineState> pipelineState = [metalDevice newComputePipelineStateWithFunction:computeFunc->function error:&error];
            if(error.code < 0){
                DEBUG_STREAM("Failed to Create Compute Pipeline State");
            };
            return std::make_shared<GEMetalComputePipelineState>(pipelineState);
        };
        SharedHandle<GEFence> makeFence(){
            auto fence = [metalDevice newFence];
            return std::make_shared<GEMetalFence>(fence);
        };
        SharedHandle<GEHeap> makeHeap(const HeapDescriptor &desc){
            MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc]init];
            
        };
        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc){
            return std::make_shared<GEMetalNativeRenderTarget>(desc.metalLayer);
        };
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor &desc){
            MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
            
            GEMetalFunction *vertexFunc = (GEMetalFunction *)desc.vertexFunc.get();
            GEMetalFunction *fragmentFunc = (GEMetalFunction *)desc.fragmentFunc.get();
            pipelineDesc.vertexFunction = vertexFunc->function;
            pipelineDesc.fragmentFunction = fragmentFunc->function;
            pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            
            NSError *error;
            id<MTLRenderPipelineState> pipelineState =  [metalDevice newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
            
            if(error.code < 0){
                DEBUG_STREAM("Failed to Create Render Pipeline State");
                exit(1);
            };
            
            return std::make_shared<GEMetalRenderPipelineState>(pipelineState);
        };
        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc){
            
        };
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
