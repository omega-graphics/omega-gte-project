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
            CPURead = 0x02,
            CPUWrite = 0x03,
            RenderTarget = 0x04
        } GETextureUsage;
        virtual void copyBytes(void *bytes,size_t len) = 0;
    };
    struct  OMEGAGTE_EXPORT TextureDescriptor {
        GETexture::GETextureType type;
        StorageOpts storage_opts;
        GETexture::GETextureUsage usage;
        unsigned width;
        unsigned height;
        unsigned depth;
    };

    struct  OMEGAGTE_EXPORT TextureSamplerDescriptor {
        
    };

_NAMESPACE_END_

#endif
