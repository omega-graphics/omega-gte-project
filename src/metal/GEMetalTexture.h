
#include "omegaGTE/GTEBase.h"
#include "omegaGTE/GETexture.h"

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

class GEMetalTexture : public GETexture {
    id<MTLTexture> texture;
    TextureDescriptor desc;
    friend class GEMetalCommandBuffer;
public:
    void copyBytes(void *bytes,size_t len) override;
    GEMetalTexture(id<MTLTexture> texture,const TextureDescriptor & desc);
};

_NAMESPACE_END_
