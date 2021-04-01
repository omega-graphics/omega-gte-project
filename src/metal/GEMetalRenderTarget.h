#include "GEMetal.h"
#include "omegaGTE/GERenderTarget.h"
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include "GEMetalTexture.h"
#include "GEMetalCommandQueue.h"


#ifndef OMEGAGTE_METAL_GEMETALRENDERTARGET_H
#define OMEGAGTE_METAL_GEMETALRENDERTARGET_H

_NAMESPACE_BEGIN_


class GEMetalNativeRenderTarget : public GENativeRenderTarget {
    SharedHandle<GEMetalCommandQueue> commandQueue;
public:
    CAMetalLayer *metalLayer;
};

class GEMetalTextureRenderTarget : public GETextureRenderTarget {
public:
    SharedHandle<GEMetalTexture> texturePtr;
};

_NAMESPACE_END_

#endif
