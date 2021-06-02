#import "GEMetal.h"
#include <sstream>
#include <memory>
#import "GEMetalCommandQueue.h"
#import "GEMetalTexture.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

    GEMetalBuffer::GEMetalBuffer(id<MTLBuffer> buffer):metalBuffer(buffer){};
    
    size_t GEMetalBuffer::size(){
        return metalBuffer.length;
    };

    void * GEMetalBuffer::data(){
        return [metalBuffer contents];
    };

    GEMetalFence::GEMetalFence(id<MTLFence> fence):metalFence(fence){};

    class GEMetalEngine : public OmegaGraphicsEngine {
        __strong id<MTLDevice> metalDevice;
    public:
        GEMetalEngine():metalDevice(MTLCreateSystemDefaultDevice()){
            DEBUG_STREAM("GEMetalEngine Successfully Created");
        };
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount) override{
            id<MTLCommandQueue> commandQueue = [metalDevice newCommandQueueWithMaxCommandBufferCount:maxBufferCount];
            return std::make_shared<GEMetalCommandQueue>(commandQueue,maxBufferCount);
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc) override{
            id<MTLBuffer> buffer = [metalDevice newBufferWithLength:desc.len options:MTLResourceStorageModeShared];
            return std::make_shared<GEMetalBuffer>(buffer);
        };
        SharedHandle<GEComputePipelineState> makeComputePipelineState(const ComputePipelineDescriptor &desc) override{
            GEMetalFunction *computeFunc = (GEMetalFunction *)desc.computeFunc.get();
            NSError *error;
            id<MTLComputePipelineState> pipelineState = [metalDevice newComputePipelineStateWithFunction:computeFunc->function error:&error];
            if(error.code < 0){
                DEBUG_STREAM("Failed to Create Compute Pipeline State");
            };
            return std::make_shared<GEMetalComputePipelineState>(pipelineState);
        };
        SharedHandle<GEFence> makeFence() override{
            auto fence = [metalDevice newFence];
            return std::make_shared<GEMetalFence>(fence);
        };
        SharedHandle<GEHeap> makeHeap(const HeapDescriptor &desc) override{
            MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc]init];
            return nullptr;
        };
        SharedHandle<GENativeRenderTarget> makeNativeRenderTarget(const NativeRenderTargetDescriptor &desc) override{
            desc.metalLayer.device = metalDevice;
            return std::make_shared<GEMetalNativeRenderTarget>(makeCommandQueue(100),desc.metalLayer);
        };
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(const RenderPipelineDescriptor &desc) override{
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
        SharedHandle<GEFunctionLibrary> loadLibrary(std::filesystem::path path,std::initializer_list<std::string> func_names) override{
            NSURL *url = [NSURL fileURLWithFileSystemRepresentation:path.c_str() isDirectory:NO relativeToURL:nil];
            NSError *error;
            id<MTLLibrary> metallib = [metalDevice newLibraryWithURL:url error:&error];
            if(error.code >= 0){
                auto funcLib = std::make_shared<GEFunctionLibrary>();
                funcLib->mtlLib = metallib;
                /// Succeeded.
                for(const std::string & func_name : func_names) {
                    NSString *nsStr = [NSString stringWithUTF8String:func_name.c_str()];
                    id <MTLFunction> mtlFunc = [metallib newFunctionWithName:nsStr];
                    auto func = new GEMetalFunction();
                    func->function = mtlFunc;
                    funcLib->functions.emplace_back(func);
                };
                return funcLib;
            }
            else {
                /// Failed
                std::ostringstream oss;
                oss << "Failed to Load MTLLibrary from path:" << path.c_str() << " " << std::flush;
                DEBUG_STREAM(oss.str());
                return nullptr;
            };
            
        };

        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc) override{
            return nullptr;
        };
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc) override{
            MTLTextureDescriptor *mtlDesc = [[MTLTextureDescriptor alloc] init];
            id<MTLTexture> texture = [metalDevice newTextureWithDescriptor:mtlDesc];
            return std::shared_ptr<GETexture>(new GEMetalTexture(texture,desc));
        };
    };


    SharedHandle<OmegaGraphicsEngine> CreateMetalEngine(){
        return std::shared_ptr<OmegaGraphicsEngine>(new GEMetalEngine());
    };
_NAMESPACE_END_
