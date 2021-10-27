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
    void setName(OmegaCommon::StrRef name) override {
        NSOBJECT_OBJC_BRIDGE(id<MTLTexture>,texture.handle()).label = [[NSString alloc] initWithUTF8String:name.data()];
    }
    void *native() override {
        return const_cast<void *>(texture.handle());
    }
    void copyBytes(void *bytes,size_t len) override;
    size_t getBytes(void *bytes, size_t bytesPerRow) override;
    explicit GEMetalTexture(const GETexture::GETextureType &type,
                   const GETexture::GETextureUsage & usage,
                   const TexturePixelFormat & pixelFormat,
                   NSSmartPtr texture);
};

_NAMESPACE_END_
