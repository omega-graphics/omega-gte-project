#include "GEMetal.h"

#include "omegaGTE/GTEBase.h"
#include "omegaGTE/GETexture.h"

#import <Metal/Metal.h>

_NAMESPACE_BEGIN_

class GEMetalTexture : public GETexture {
    NSSmartPtr texture;
    NSSmartPtr resourceBarrier;

    bool needsBarrier = false;

    friend class GEMetalCommandBuffer;
public:
    void copyBytes(void *bytes,size_t len) override;
    explicit GEMetalTexture(const GETexture::GETextureType &type,
                   const GETexture::GETextureUsage & usage,
                   const TexturePixelFormat & pixelFormat,
                   NSSmartPtr texture);
};

_NAMESPACE_END_
