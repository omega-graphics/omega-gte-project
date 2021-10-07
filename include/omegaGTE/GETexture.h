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
            ToGPU = 0x00,
            FromGPU = 0x01,
            GPUAccessOnly = 0x02,
            RenderTarget = 0x03,
            MSResolveSrc = 0x04
        } GETextureUsage;
    protected:
        GETextureType type;
        GETextureUsage usage;
        TexturePixelFormat pixelFormat;
        bool checkIfCanWrite();
        explicit GETexture(
                const GETextureType & type,
                  const GETextureUsage & usage,
                  const TexturePixelFormat & pixelFormat);
    public:
        virtual void copyBytes(void *bytes,size_t bytesPerRow) = 0;
        virtual ~GETexture() = default;
    };

    struct OMEGAGTE_EXPORT TextureRegion {
        unsigned x,y,z;
        unsigned w,h,d;
    };


    struct  OMEGAGTE_EXPORT TextureDescriptor {
        GETexture::GETextureType type;
        StorageOpts storage_opts;
        GETexture::GETextureUsage usage = GETexture::ToGPU;
        TexturePixelFormat pixelFormat = TexturePixelFormat::RGBA8Unorm;
        unsigned width;
        unsigned height;
        unsigned depth = 1;
        unsigned mipLevels = 1;
        unsigned sampleCount = 1;
    };

_NAMESPACE_END_

#endif
