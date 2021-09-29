#import "GEMetal.h"
#include <sstream>
#include <memory>
#import "GEMetalCommandQueue.h"
#import "GEMetalTexture.h"
#import "GEMetalRenderTarget.h"
#import "GEMetalPipeline.h"
#include <cassert>
#include <fstream>
#include <cstdlib>
#include <cstring>

#include "OmegaGTE.h"

#import <Metal/Metal.h>

#if !__has_attribute(ext_vector_type)
#pragma error("Requires vector types")
#endif

_NAMESPACE_BEGIN_

    struct GTEMetalDevice : public GTEDevice {
        __strong id<MTLDevice> device;
        GTEMetalDevice(Type type,const char *name,GTEDeviceFeatures & features,id<MTLDevice> _device): GTEDevice(type,name,features),device(_device){}
    };

    /// GTE Device Enumerate
    OmegaCommon::Vector<SharedHandle<GTEDevice>> enumerateDevices(){
        OmegaCommon::Vector<SharedHandle<GTEDevice>> devs;
        NSArray<id<MTLDevice>> *mtlDevices = MTLCopyAllDevices();

        for(id<MTLDevice> dev in mtlDevices){
            GTEDeviceFeatures features {
                    (bool)dev.supportsRaytracing
            };
            GTEDevice::Type type;
            if(dev.lowPower){
                type = GTEDevice::Integrated;
            }
            else {
                type = GTEDevice::Discrete;
            }
            devs.push_back(SharedHandle<GTEDevice>(new GTEMetalDevice {type,dev.name.UTF8String,features,dev}));
        }
        return devs;
    }



    /// =========================================================>



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

    GEMetalBuffer::GEMetalBuffer(NSSmartPtr & buffer,NSSmartPtr &layoutDesc):metalBuffer(buffer), layoutDesc(layoutDesc){

    };
    
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



    GEMetalFence::GEMetalFence(NSSmartPtr & event):metalEvent(event){};

    GEMetalSamplerState::GEMetalSamplerState(NSSmartPtr &samplerState): samplerState(samplerState) {

    }

    class GEMetalEngine : public OmegaGraphicsEngine {
        NSSmartPtr metalDevice;
        SharedHandle<GTEShader> _loadShaderFromDesc(omegasl_shader *shaderDesc) override {
            auto data = dispatch_data_create(shaderDesc->data,shaderDesc->dataSize,nullptr,DISPATCH_DATA_DESTRUCTOR_DEFAULT);
            NSError *error;
            NSSmartPtr library = NSObjectHandle {NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newLibraryWithData:data error:&error]};
            
            NSString *str = [[NSString alloc] initWithUTF8String:shaderDesc->name];
            NSLog(@"Loading Function %@",str);
            NSSmartPtr func = NSObjectHandle {NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLLibrary>,library.handle()) newFunctionWithName:str] };
            auto _shader = new GEMetalShader(library,func);
            _shader->internal = *shaderDesc;
            return SharedHandle<GTEShader>(_shader);
        }
    public:
        GEMetalEngine(SharedHandle<GTEDevice> & __device){
            __strong id<MTLDevice> device = ((GTEMetalDevice *)__device.get())->device;
            if(device == nil){
                NSLog(@"Metal is not supported on this device! Exiting...");
                exit(1);
            }
            MTLCaptureManager *manager = [MTLCaptureManager sharedCaptureManager];

            MTLCaptureDescriptor *captureDesc = [[MTLCaptureDescriptor alloc] init];
            captureDesc.captureObject = device;
            captureDesc.destination = MTLCaptureDestinationGPUTraceDocument;
            captureDesc.outputURL = [NSURL fileURLWithPath:@"./2DTest.gputrace"];
            NSError *error;
            BOOL res = [manager startCaptureWithDescriptor:captureDesc error:&error];

            if(!res){
                NSLog(@"Failed to Start GPU Capture. %@",error);
            }

            metalDevice = NSObjectHandle {NSOBJECT_CPP_BRIDGE device};
            DEBUG_STREAM("GEMetalEngine Successfully Created");
            
        };
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount) override{
            metalDevice.assertExists();
            NSSmartPtr commandQueue ({NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newCommandQueueWithMaxCommandBufferCount:maxBufferCount]});
            return std::shared_ptr<GECommandQueue>(new GEMetalCommandQueue(commandQueue,maxBufferCount));
        };
        SharedHandle<GEBuffer> makeBuffer(const BufferDescriptor &desc) override{
            metalDevice.assertExists();
            MTLBufferLayoutDescriptor *descriptor = [[MTLBufferLayoutDescriptor alloc] init];
            descriptor.stride = desc.objectStride;
            /// Only defines object stride.
            id<MTLBuffer> mtlBuffer = [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newBufferWithLength:desc.len options:MTLResourceStorageModeShared];

            NSSmartPtr buffer ({NSOBJECT_CPP_BRIDGE mtlBuffer}),
            layoutDesc(NSObjectHandle {NSOBJECT_CPP_BRIDGE descriptor});
            return std::shared_ptr<GEBuffer>(new GEMetalBuffer(buffer,layoutDesc));
        };
        SharedHandle<GEComputePipelineState> makeComputePipelineState(ComputePipelineDescriptor &desc) override{
            metalDevice.assertExists();
//
//
//
            auto & threadgroup_desc = desc.computeFunc->internal.threadgroupDesc;

            MTLComputePipelineDescriptor *pipelineDescriptor = [[MTLComputePipelineDescriptor alloc] init];
            pipelineDescriptor.maxTotalThreadsPerThreadgroup = (threadgroup_desc.x * threadgroup_desc.y * threadgroup_desc.z);


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
             NSSmartPtr fence = NSObjectHandle {NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newEvent]};
             return SharedHandle<GEFence>(new GEMetalFence(fence));
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
            
            auto *vertexFunc = (GEMetalShader *)desc.vertexFunc.get();
            auto *fragmentFunc = (GEMetalShader *)desc.fragmentFunc.get();
            vertexFunc->function.assertExists();
            fragmentFunc->function.assertExists();
//            pipelineDesc.label = @"RENDER PIPELINE";
            pipelineDesc.vertexFunction = NSOBJECT_OBJC_BRIDGE(id<MTLFunction>,vertexFunc->function.handle());
            pipelineDesc.fragmentFunction = NSOBJECT_OBJC_BRIDGE(id<MTLFunction>,fragmentFunc->function.handle());
            pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            
            NSError *error;
            NSSmartPtr pipelineState =  NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newRenderPipelineStateWithDescriptor:pipelineDesc error:&error]};
            
            if(pipelineState.handle() == nil || error.code < 0){
                DEBUG_STREAM("Failed to Create Render Pipeline State");
                exit(1);
            };
            
            return std::shared_ptr<GERenderPipelineState>(new GEMetalRenderPipelineState(desc.vertexFunc,desc.fragmentFunc,pipelineState));
        };

        SharedHandle<GETextureRenderTarget> makeTextureRenderTarget(const TextureRenderTargetDescriptor &desc) override {
            metalDevice.assertExists();
            SharedHandle<GETexture> texture;
            if(desc.renderToExistingTexture){
                texture = texture;
            }
            else {
                TextureDescriptor textureDescriptor {
                    GETexture::Texture2D,
                    Shared,
                    GETexture::RenderTarget,
                    TexturePixelFormat::RGBA8Unorm,
                    (unsigned int)desc.rect.w,
                    (unsigned int)desc.rect.h,0};

                texture = makeTexture(textureDescriptor);
            }
            auto commandQueue = makeCommandQueue(100);
            return SharedHandle<GETextureRenderTarget>(new GEMetalTextureRenderTarget(texture,commandQueue));
        };
        SharedHandle<GETexture> makeTexture(const TextureDescriptor &desc) override{
            assert(desc.sampleCount >= 1 && "Can only create textures with 1 or more samples");
            metalDevice.assertExists();
            MTLTextureDescriptor *mtlDesc = [[MTLTextureDescriptor alloc] init];
            MTLTextureType textureType;
            switch (desc.type) {
                case GETexture::Texture1D : {
                    textureType = MTLTextureType1D;
                    break;
                }
                case GETexture::Texture2D : {
                    if(desc.sampleCount > 1){
                        textureType = MTLTextureType2DMultisample;
                    }
                    else {
                        textureType = MTLTextureType2D;
                    }
                    break;
                }
                case GETexture::Texture3D : {
                    textureType = MTLTextureType3D;
                    break;
                }
            }
            mtlDesc.textureType = textureType;
            mtlDesc.width = desc.width;
            mtlDesc.height = desc.height;
            mtlDesc.sampleCount = desc.sampleCount;
            mtlDesc.depth = desc.depth;
            mtlDesc.arrayLength = 1;
            mtlDesc.storageMode = MTLStorageModeShared;

            MTLPixelFormat pixelFormat;
            switch (desc.pixelFormat) {
                case TexturePixelFormat::RGBA8Unorm : {
                    pixelFormat = MTLPixelFormatRGBA8Unorm;
                    break;
                }
                case TexturePixelFormat::RGBA16Unorm : {
                    pixelFormat = MTLPixelFormatRGBA16Unorm;
                    break;
                }
                case TexturePixelFormat::RGBA8Unorm_SRGB : {
                    pixelFormat = MTLPixelFormatRGBA8Unorm_sRGB;
                    break;
                }
            }

            mtlDesc.pixelFormat = pixelFormat;

            NSSmartPtr texture = NSObjectHandle {NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newTextureWithDescriptor:mtlDesc]};
            return std::shared_ptr<GETexture>(new GEMetalTexture(texture,desc));
        };
        SharedHandle<GESamplerState> makeSamplerState(const SamplerDescriptor &desc) override {
            MTLSamplerDescriptor *mtlSamplerDescriptor = [[MTLSamplerDescriptor alloc] init];

            switch (desc.filter) {
                case SamplerDescriptor::Filter::Linear : {
                    mtlSamplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
                    mtlSamplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
                    mtlSamplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
                    break;
                }
            }

            mtlSamplerDescriptor.maxAnisotropy = desc.maxAnisotropy;

            MTLSamplerAddressMode samplerAddressMode = MTLSamplerAddressModeClampToZero;
            switch (desc.uAddressMode) {
                case SamplerDescriptor::AddressMode::Wrap : {
                    samplerAddressMode = MTLSamplerAddressModeRepeat;
                    break;
                }
                case SamplerDescriptor::AddressMode::ClampToEdge : {
                    samplerAddressMode = MTLSamplerAddressModeClampToEdge;
                    break;
                }
                case SamplerDescriptor::AddressMode::MirrorWrap : {
                    samplerAddressMode = MTLSamplerAddressModeMirrorRepeat;
                    break;
                }
                case SamplerDescriptor::AddressMode::MirrorClampToEdge : {
                    samplerAddressMode = MTLSamplerAddressModeMirrorClampToEdge;
                    break;
                }
            }
            mtlSamplerDescriptor.sAddressMode = samplerAddressMode;
            switch (desc.vAddressMode) {
                case SamplerDescriptor::AddressMode::Wrap : {
                    samplerAddressMode = MTLSamplerAddressModeRepeat;
                    break;
                }
                case SamplerDescriptor::AddressMode::ClampToEdge : {
                    samplerAddressMode = MTLSamplerAddressModeClampToEdge;
                    break;
                }
                case SamplerDescriptor::AddressMode::MirrorWrap : {
                    samplerAddressMode = MTLSamplerAddressModeMirrorRepeat;
                    break;
                }
                case SamplerDescriptor::AddressMode::MirrorClampToEdge : {
                    samplerAddressMode = MTLSamplerAddressModeMirrorClampToEdge;
                    break;
                }
            }
            mtlSamplerDescriptor.tAddressMode = samplerAddressMode;
            switch (desc.wAddressMode) {
                case SamplerDescriptor::AddressMode::Wrap : {
                    samplerAddressMode = MTLSamplerAddressModeRepeat;
                    break;
                }
                case SamplerDescriptor::AddressMode::ClampToEdge : {
                    samplerAddressMode = MTLSamplerAddressModeClampToEdge;
                    break;
                }
                case SamplerDescriptor::AddressMode::MirrorWrap : {
                    samplerAddressMode = MTLSamplerAddressModeMirrorRepeat;
                    break;
                }
                case SamplerDescriptor::AddressMode::MirrorClampToEdge : {
                    samplerAddressMode = MTLSamplerAddressModeMirrorClampToEdge;
                    break;
                }
            }
            mtlSamplerDescriptor.rAddressMode = samplerAddressMode;

            NSSmartPtr samplerState = NSObjectHandle{NSOBJECT_CPP_BRIDGE [NSOBJECT_OBJC_BRIDGE(id<MTLDevice>,metalDevice.handle()) newSamplerStateWithDescriptor:mtlSamplerDescriptor]};

            return SharedHandle<GESamplerState>(new GEMetalSamplerState {samplerState});
        }
    };


    SharedHandle<OmegaGraphicsEngine> CreateMetalEngine(SharedHandle<GTEDevice> & device){
        return std::shared_ptr<OmegaGraphicsEngine>(new GEMetalEngine(device));
    };
_NAMESPACE_END_
