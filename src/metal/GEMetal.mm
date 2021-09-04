#import "GEMetal.h"
#include <sstream>
#include <memory>
#import "GEMetalCommandQueue.h"
#import "GEMetalTexture.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"
#include <cassert>
#include <fstream>

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

    simd_float2 float2(const FVec<2> &mat){
        return simd::make_float2(mat[0][0],mat[1][0]);
    };
    simd_float3 float3(const FVec<3> &mat){
        return simd::make_float3(mat[0][0],mat[1][0],mat[2][0]);
    };
    simd_float4 float4(const FVec<4> &mat){
        return simd::make_float4(mat[0][0],mat[1][0],mat[2][0],mat[3][0]);
    };

    simd_double2 double2(const DVec<2> &mat){
        return simd::make_double2(mat[0][0],mat[1][0]);
    };
    simd_double3 double3(const DVec<3> &mat){
        return simd::make_double3(mat[0][0],mat[1][0],mat[2][0]);
    };
    simd_double4 double4(const DVec<4> &mat){
        return simd::make_double4(mat[0][0],mat[1][0],mat[2][0],mat[3][0]);
    };

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

    GEMetalBuffer::~GEMetalBuffer(){

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
            return std::shared_ptr<GECommandQueue>(new GEMetalCommandQueue(commandQueue,maxBufferCount));
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc) override{
            metalDevice.assertExists();
            NSSmartPtr buffer ({NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newBufferWithLength:desc.len options:MTLResourceStorageModeShared]});
            return std::shared_ptr<GEBuffer>(new GEMetalBuffer(buffer));
        };
        SharedHandle<GEComputePipelineState> makeComputePipelineState(ComputePipelineDescriptor &desc) override{
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
            return std::shared_ptr<GENativeRenderTarget>(new GEMetalNativeRenderTarget(commandQueue,desc.metalLayer));
        };
        SharedHandle<GERenderPipelineState> makeRenderPipelineState(RenderPipelineDescriptor &desc) override{
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
            
            if(pipelineState.handle() == nil){
                DEBUG_STREAM("Failed to Create Render Pipeline State");
                exit(1);
            };
            
            return std::shared_ptr<GERenderPipelineState>(new GEMetalRenderPipelineState(pipelineState));
        };
        // SharedHandle<GEFunctionLibrary> loadStdShaderLibrary() override{
 //            /// NOTE: This a temporary fix.. Please optimize!
 //            NSString *shaderMapURL = [[NSBundle bundleWithIdentifier:@"org.omegagraphics.OmegaGTE"] pathForResource:@"std" ofType:@"shadermap" inDirectory:@"stdshaderlib"];
 //            NSLog(@"Resource Path: %@",shaderMapURL);
 //            return loadShaderLibrary(std::string(shaderMapURL.UTF8String));
 //            // return loadShaderLibrary(std::string(shaderMapURL.UTF8String));
 //        };
        // SharedHandle<GEFunctionLibrary> loadShaderLibrary(FS::Path path) override{
//             /// Load OmegaSL Shadermap
//             std::ifstream in(path.str(),std::ios::binary | std::ios::in);
//
//             if(in.is_open()){
//                 SharedHandle<GEFunctionLibrary> funcLibrary = std::make_shared<GEFunctionLibrary>();
//                 unsigned b;
//                 in.read((char *)&b,sizeof(b));
//                 DEBUG_STREAM("LENGTH:" << b);
//                 while(b > 0){
//                     unsigned entNameLen;
//                     in.read((char *)&entNameLen,sizeof(entNameLen));
//                     char *name = new char[entNameLen];
//                     in.read(name,sizeof(char) * entNameLen);
//                     std::string_view str(name,entNameLen);
//                     NSString *file = [[NSString alloc] initWithUTF8String:path.str().c_str()];
//                     NSLog(@"FILE LOC:%@",file);
//                     NSError *error;
//                     NSSmartPtr mtlLibrary = NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newLibraryWithFile:file error:&error] };
//
//                     if(mtlLibrary.handle() == nil){
//                         DEBUG_STREAM("Failed to Load Metal Library From:" << path.str());
//                     };
//
//                     unsigned shaderCount;
//                     in.read((char *)&shaderCount,sizeof(shaderCount));
//                     DEBUG_STREAM("Metal Lib ShaderCount:" << shaderCount);
//                     while(shaderCount > 0){
//                         unsigned funcNameLen;
//                         in.read((char *)&funcNameLen,sizeof(funcNameLen));
//                         char *name = new char[funcNameLen + 1];
//                         in.read(name,sizeof(char) * funcNameLen);
//                         name[funcNameLen] = '\0';
//                         std::string_view func_name(name,funcNameLen);
//
//                         std::cout << "FUNC:" << func_name << std::endl;
//
//                         NSString *str = [[NSString alloc] initWithUTF8String:func_name.data()];
//
//                         NSSmartPtr mtlFunc = NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLLibrary>,mtlLibrary.handle()) newFunctionWithName:str] };
//
//                         if(mtlFunc.handle() == nil){
//                             DEBUG_STREAM("Failed to Load Metal Shader \"" << func_name << "\" from path:" << path.str());
//                         };
//                         funcLibrary->functions.insert(std::make_pair(func_name,std::make_shared<GEMetalFunction>(mtlFunc)));
//
//                         --shaderCount;
//                     };
//
//                     --b;
//                 };
//                 in.close();
//                 return funcLibrary;
//             }
//             else {
//                 return nullptr;
//             };
//         };

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
