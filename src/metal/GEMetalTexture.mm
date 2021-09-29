#import "GEMetalTexture.h"

_NAMESPACE_BEGIN_

GEMetalTexture::GEMetalTexture(NSSmartPtr texture,const TextureDescriptor & desc):texture(texture),desc(desc){
    
};

void GEMetalTexture::copyBytes(void *bytes, size_t len){
   
    [NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,texture.handle()) replaceRegion:MTLRegionMake2D(0,0,desc.width,desc.height) mipmapLevel:0 withBytes:bytes bytesPerRow:len * 4 * 8];
};


_NAMESPACE_END_
