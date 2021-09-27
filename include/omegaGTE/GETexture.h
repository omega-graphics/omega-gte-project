#include "GTEBase.h"
#include "GE.h"

#ifndef OMEGAGTE_GETEXTURE_H
#define OMEGAGTE_GETEXTURE_H

_NAMESPACE_BEGIN_

    class  OMEGAGTE_EXPORT GETexture {
    public:
        typedef enum : unsigned char {
            Texture2D,
            Texture3D,
        } GETextureType;
        typedef enum : unsigned char {
            GPURead = 0x00,
            GPUWrite = 0x01,
            RenderTarget = 0x02
        } GETextureUsage;
    protected:
        GETextureType type;
        GETextureUsage usage;
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
        GETexture::GETextureUsage usage;
        unsigned width;
        unsigned height;
        unsigned depth;
    };

_NAMESPACE_END_

#endif
