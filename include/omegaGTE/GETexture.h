#include "GTEBase.h"
#include "GE.h"

#ifndef OMEGAGTE_GETEXTURE_H
#define OMEGAGTE_GETEXTURE_H

_NAMESPACE_BEGIN_

    enum class TexturePixelFormat : int {
        RGBA8Unorm,
        RGBA16Unorm,
        RGBA8Unorm_SRGB
    };

    class  OMEGAGTE_EXPORT GETexture {
    public:
        typedef enum : unsigned char {
            Texture1D,
            Texture2D,
            Texture3D,
        } GETextureType;
        typedef enum : unsigned char {
            GPURead = 0x00,
            GPUWrite = 0x01,
            RenderTarget = 0x02,
            MSResolveDest = 0x03
        } GETextureUsage;
    protected:
        GETextureType type;
        GETextureUsage usage;
        TexturePixelFormat pixelFormat;
    public:
        virtual void copyBytes(void *bytes,size_t len) = 0;
    };

    struct OMEGAGTE_EXPORT TextureRegion {
        unsigned x,y,z;
        unsigned w,h,d;
    };


    struct  OMEGAGTE_EXPORT TextureDescriptor {
        GETexture::GETextureType type;
        StorageOpts storage_opts;
        GETexture::GETextureUsage usage = GETexture::GPURead;
        TexturePixelFormat pixelFormat = TexturePixelFormat::RGBA8Unorm;
        unsigned width;
        unsigned height;
        unsigned depth;
        unsigned mipLevels = 0;
        unsigned sampleCount = 1;
    };

_NAMESPACE_END_

#endif
