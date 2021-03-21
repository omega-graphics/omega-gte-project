#include "GTEBase.h"

#ifndef OMEGAGRAPHICSENGINE_GETEXTURE_H
#define OMEGAGRAPHICSENGINE_GETEXTURE_H

_NAMESPACE_BEGIN_

    typedef enum : unsigned char {
        Texture2D,
        Texture3D,
        Texture2DMS,
        Texture3DMS
    } GETextureType;

    struct TextureDescriptor {
        GETextureType type;
    };

    class GETexture {
        
    };
_NAMESPACE_END_

#endif