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

#if !__has_attribute(ext_vector_type)
#pragma error("Requires vector types")
#endif

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

    GEMetalBuffer::~GEMetalBuffer(){

    };

    /// @brief Metal Buffer Reader/Writer.

    typedef unsigned char MTLByte;

    class GEMetalBufferWriter : public GEBufferWriter {
        GEMetalBuffer *buffer_ = nil;
        MTLByte *_data_ptr = nullptr;
        size_t currentOffset = 0;
    public:
        GEMetalBufferWriter() = default;
        void setOutputBuffer(SharedHandle<GEBuffer> &buffer) override {
            buffer_= (GEMetalBuffer *)buffer.get();
            currentOffset = 0;
            _data_ptr = (MTLByte *)[NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,buffer_->metalBuffer.handle()) contents];;
        }
        void structBegin() override {

        }
        void writeFloat(float &v) override {
            memcpy(_data_ptr + currentOffset,&v,sizeof(v));
            currentOffset += sizeof(v);
        }
        void writeFloat2(FVec<2> &v) override {
            auto _v = float2(v);
            memcpy(_data_ptr + currentOffset,&_v,sizeof(_v));
            currentOffset += sizeof(_v);
        }
        void writeFloat3(FVec<3> &v) override {
            auto _v = float3(v);
            memcpy(_data_ptr + currentOffset,&_v,sizeof(_v));
            currentOffset += sizeof(_v);
        }
        void writeFloat4(FVec<4> &v) override {
            auto _v = float4(v);
            memcpy(_data_ptr + currentOffset,&_v,sizeof(_v));
            currentOffset += sizeof(_v);
        }
        void structEnd() override {

        }
        void finish() override {
            buffer_ = nil;
            _data_ptr = nullptr;
        }
    };

    SharedHandle<GEBufferWriter> GEBufferWriter::Create() {
        return std::shared_ptr<GEBufferWriter>(new GEMetalBufferWriter());
    }

    class GEMetalBufferReader : public GEBufferReader {
        GEMetalBuffer *buffer_ = nullptr;
        MTLByte *_data_ptr = nullptr;
        size_t currentOffset = 0;
    public:
        GEMetalBufferReader() = default;
        void setInputBuffer(SharedHandle<GEBuffer> &buffer) override {
            currentOffset = 0;
            buffer_= (GEMetalBuffer *)buffer.get();
            _data_ptr = (MTLByte *)[NSOBJECT_OBJC_BRIDGE(id<MTLBuffer>,buffer_->metalBuffer.handle()) contents];
        }
        void structBegin() override {

        }
        void getFloat(float &v) override {
            memcpy(&v,_data_ptr + currentOffset,sizeof(v));
            currentOffset += sizeof(v);
        }
        void getFloat2(FVec<2> &v) override {
            simd_float2 _v;
            memcpy(&_v,_data_ptr + currentOffset,sizeof(_v));
            currentOffset += sizeof(_v);
            
            v[0][0] = _v.x;
            v[1][0] = _v.y;
        }
        void getFloat3(FVec<3> &v) override {
            simd_float3 _v;
            memcpy(&_v,_data_ptr + currentOffset,sizeof(_v));
            currentOffset += sizeof(_v);

            v[0][0] = _v.x;
            v[1][0] = _v.y;
            v[2][0] = _v.z;
        }
        void getFloat4(FVec<4> &v) override {
            simd_float4 _v;
            memcpy(&_v,_data_ptr + currentOffset,sizeof(_v));
            currentOffset += sizeof(_v);

            v[0][0] = _v.x;
            v[1][0] = _v.y;
            v[2][0] = _v.z;
            v[3][0] = _v.w;
        }
        void structEnd() override {

        }
        void finish() override {
            _data_ptr = nullptr;
            buffer_ = nullptr;
        }
    };

    SharedHandle<GEBufferReader> GEBufferReader::Create() {
        return std::shared_ptr<GEBufferReader>(new GEMetalBufferReader());
    }



    GEMetalFence::GEMetalFence(NSSmartPtr & fence):metalFence(fence){};

    class GEMetalEngine : public OmegaGraphicsEngine {
        NSSmartPtr metalDevice;
        SharedHandle<GTEShader> _loadShaderFromDesc(omegasl_shader *shaderDesc) override {
            auto data = dispatch_data_create(shaderDesc->data,shaderDesc->dataSize,nullptr,DISPATCH_DATA_DESTRUCTOR_DEFAULT);
            NSError *error;
            NSSmartPtr library = NSObjectHandle {NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newLibraryWithData:data error:&error]};
            NSSmartPtr func = NSObjectHandle {NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLLibrary>,library.handle()) newFunctionWithName:[[NSString alloc] initWithUTF8String:shaderDesc->name]] };
            auto _shader = new GEMetalShader(library,func);
            _shader->internal = *shaderDesc;
            return SharedHandle<GTEShader>(_shader);
        }
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
            metalDevice.assertExists();
            /// TODO: Make MTLComputeFunctionReflection from omegasl_shader desc.

            MTLComputePipelineDescriptor *pipelineDescriptor = [[MTLComputePipelineDescriptor alloc] init];

            GEMetalShader *computeShader = (GEMetalShader *)desc.computeFunc.get();
            computeShader->function.assertExists();

            pipelineDescriptor.computeFunction = NSOBJECT_OBJC_BRIDGE(id<MTLFunction>,computeShader->function.handle());

            NSError *error;
            NSSmartPtr pipelineState =  NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newComputePipelineStateWithDescriptor:pipelineDescriptor options:MTLPipelineOptionNone reflection:nil error:&error]};

            if(pipelineState.handle() == nil){
                DEBUG_STREAM("Failed to Create Compute Pipeline State");
                exit(1);
            };

            return std::shared_ptr<GEComputePipelineState>(new GEMetalComputePipelineState(desc.computeFunc,pipelineState));
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
            
            GEMetalShader *vertexFunc = (GEMetalShader *)desc.vertexFunc.get();
            GEMetalShader *fragmentFunc = (GEMetalShader *)desc.fragmentFunc.get();
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
            
            return std::shared_ptr<GERenderPipelineState>(new GEMetalRenderPipelineState(desc.vertexFunc,desc.fragmentFunc,pipelineState));
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
