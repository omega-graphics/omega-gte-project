#import "GEMetal.h"
#include <sstream>
#include <memory>
#import "GEMetalCommandQueue.h"
#import "GEMetalTexture.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"
#include <cassert>

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

    NSSmartPtr::NSSmartPtr(const NSObjectHandle & handle):data(handle.data){

    };

    void NSSmartPtr::assertExists(){
        assert(data);
    };

    GEMetalBuffer::GEMetalBuffer(NSSmartPtr & buffer):metalBuffer(buffer){};
    
    size_t GEMetalBuffer::size(){
        metalBuffer.assertExists();
        return NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,metalBuffer.handle()).length;
    };

    void * GEMetalBuffer::data(){
        metalBuffer.assertExists();
        return [NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,metalBuffer.handle()) contents];
    };

    GEMetalFence::GEMetalFence(NSSmartPtr & fence):metalFence(fence){};

    class GEMetalEngine : public OmegaGraphicsEngine {
        NSSmartPtr metalDevice;
    public:
        GEMetalEngine():metalDevice({NSOBJECT_CPP_BRIDGE MTLCreateSystemDefaultDevice()}){
            DEBUG_STREAM("GEMetalEngine Successfully Created");
        };
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount) override{
            metalDevice.assertExists();
            NSSmartPtr commandQueue ({NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newCommandQueueWithMaxCommandBufferCount:maxBufferCount]});
            return std::make_shared<GEMetalCommandQueue>(commandQueue,maxBufferCount);
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc) override{
            metalDevice.assertExists();
            NSSmartPtr buffer ({NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newBufferWithLength:desc.len options:MTLResourceStorageModeShared]});
            return std::make_shared<GEMetalBuffer>(buffer);
        };
        SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor &desc) override{
            // GEMetalFunction *computeFunc = (GEMetalFunction *)desc.computeFunc.get();
            // NSError *error;
            // id<MTLComputePipelineState> pipelineState = [metalDevice newComputePipelineStateWithFunction:computeFunc->function error:&error];
            // if(error.code < 0){
            //     DEBUG_STREAM("Failed to Create Compute Pipeline State");
            // };
            // return std::make_shared<GEMetalComputePipelineState>(pipelineState);
            return nullptr;
        };
        SharedHandle<GEFence> makeFence() override{
            // auto fence = [metalDevice newFence];
            // return std::make_shared<GEMetalFence>(fence);
            return nullptr;
        };
        SharedHandle<GEHeap> makeHeap(const HeapDescriptor &desc) override{
            MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
            return nullptr;
        };
        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc) override{
            metalDevice.assertExists();
            desc.metalLayer.device = NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle());
            auto commandQueue = makeCommandQueue(100);
            return std::make_shared<GEMetalNativeRenderTarget>(commandQueue,desc.metalLayer);
        };
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor &desc) override{
            metalDevice.assertExists();
            MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
            
            GEMetalFunction *vertexFunc = (GEMetalFunction *)desc.vertexFunc.get();
            GEMetalFunction *fragmentFunc = (GEMetalFunction *)desc.fragmentFunc.get();
            vertexFunc->function.assertExists();
            fragmentFunc->function.assertExists();
            pipelineDesc.vertexFunction = NSOBJECT_OBJC_BRIDGE(id<MTLFunction>,vertexFunc->function.handle());
            pipelineDesc.fragmentFunction = NSOBJECT_OBJC_BRIDGE(id<MTLFunction>,fragmentFunc->function.handle());
            pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            
            NSError *error;
            NSSmartPtr pipelineState =  NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newRenderPipelineStateWithDescriptor:pipelineDesc error:&error]};
            
            if(error.code < 0){
                DEBUG_STREAM("Failed to Create Render Pipeline State");
                exit(1);
            };
            
            return std::make_shared<GEMetalRenderPipelineState>(pipelineState);
        };
        SharedHandle<GEFunctionLibrary> loadShaderLibrary(std::filesystem::path path) override{
            /// Load OmegaSL Shadermap

            
        };

        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc) override{
            return nullptr;
        };
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc) override{
            metalDevice.assertExists();
            MTLTextureDescriptor *mtlDesc = [[MTLTextureDescriptor alloc] init];
            id<MTLTexture> texture = [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newTextureWithDescriptor:mtlDesc];
            return std::shared_ptr<GETexture>(new GEMetalTexture(texture,desc));
        };
    };


    SharedHandle<OmegaGraphicsEngine> CreateMetalEngine(){
        return std::shared_ptr<OmegaGraphicsEngine>(new GEMetalEngine());
    };
_NAMESPACE_END_
