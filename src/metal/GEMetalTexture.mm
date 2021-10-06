#import "GEMetalTexture.h"

_NAMESPACE_BEGIN_

GEMetalTexture::GEMetalTexture(const GETexture::GETextureType &type,
                               const GETexture::GETextureUsage & usage,
                               const TexturePixelFormat & pixelFormat,NSSmartPtr texture): GETexture(type,usage,pixelFormat),texture(texture){
    id<MTLDevice> device = NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,texture.handle()).device;
    resourceBarrier = NSObjectHandle {NSOBJECT_CPP_BRIDGE [device newFence]};
};

void GEMetalTexture::copyBytes(void *bytes, size_t len){
    auto width = NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,texture.handle()).width;
    auto height = NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,texture.handle()).height;
    [NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,texture.handle()) replaceRegion:MTLRegionMake2D(0,0,width,height) mipmapLevel:0 withBytes:bytes bytesPerRow:len * 4];
};


_NAMESPACE_END_
