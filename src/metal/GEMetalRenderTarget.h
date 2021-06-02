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
    __strong id<CAMetalDrawable> currentDrawable;
public:
    GEMetalNativeRenderTarget(SharedHandle<GECommandQueue> commandQueue,CAMetalLayer *metalLayer);
    CGSize drawableSize;
    SharedHandle<CommandBuffer> commandBuffer();
    id<CAMetalDrawable> getDrawable();
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
