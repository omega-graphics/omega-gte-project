#import "GEMetalTexture.h"

_NAMESPACE_BEGIN_

GEMetalTexture::GEMetalTexture(id<MTLTexture> texture,const TextureDescriptor & desc):texture(texture),desc(desc){
    
};


_NAMESPACE_END_
