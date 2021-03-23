
#include "omegaGTE/GTEBase.h"
#include "omegaGTE/GETexture.h"

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

class GEMetalTexture : public GETexture {
    id<MTLTexture> texture;
    TextureDescriptor desc;
public:
    GEMetalTexture(id<MTLTexture> texture,TextureDescriptor & desc);
};

_NAMESPACE_END_