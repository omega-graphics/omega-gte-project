#import "GEMetalTexture.h"

_NAMESPACE_BEGIN_

GEMetalTexture::GEMetalTexture(id<MTLTexture> texture,const TextureDescriptor & desc):texture(texture),desc(desc){
    
};

void GEMetalTexture::copyBytes(void *bytes, size_t len){
   
    [texture replaceRegion:MTLRegionMake2D(0,0,desc.width,desc.height) mipmapLevel:0 withBytes:bytes bytesPerRow:len * 4 * 8];
};


_NAMESPACE_END_
