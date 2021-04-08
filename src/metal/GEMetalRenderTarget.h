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
    CAMetalLayer *metalLayer;
public:
    GEMetalNativeRenderTarget(CAMetalLayer *metalLayer);
    CGSize *drawableSize;
    id<CAMetalDrawable> currentDrawable;
    SharedHandle<CommandBuffer> commandBuffer();
    void commitAndPresent();
    void reset();
};

class GEMetalTextureRenderTarget : public GETextureRenderTarget {
    SharedHandle<GEMetalCommandQueue> commandQueue;
public:
    GEMetalTextureRenderTarget(SharedHandle<GEMetalTexture> texture);
    SharedHandle<GEMetalTexture> texturePtr;
    SharedHandle<CommandBuffer> commandBuffer();
    void commit();
};

_NAMESPACE_END_

#endif
