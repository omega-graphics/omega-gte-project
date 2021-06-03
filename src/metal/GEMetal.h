#include "omegaGTE/GE.h"
#include <iostream>

#ifndef OMEGAGTE_METAL_GEMETAL_H
#define OMEGAGTE_METAL_GEMETAL_H

#if defined(TARGET_METAL) && defined(__OBJC__)
@protocol MTLBuffer;
@protocol MTLFence;
#endif

_NAMESPACE_BEGIN_

#if defined(TARGET_METAL) && defined(__OBJC__)

class GEMetalBuffer : public GEBuffer {
public:
    id<MTLBuffer> metalBuffer;
    size_t size();
    void * data();
    GEMetalBuffer(id<MTLBuffer> buffer);
};

class GEMetalFence : public GEFence {
public:
    id<MTLFence> metalFence;
    GEMetalFence(id<MTLFence> fence);
};

#endif

    SharedHandle<OmegaGraphicsEngine> CreateMetalEngine();
_NAMESPACE_END_

#endif


