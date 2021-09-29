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

struct NSObjectHandle {
    const void *data;
};

class NSSmartPtr {
    const void * data = nullptr;
public:
    NSSmartPtr() = default;
    NSSmartPtr(const NSObjectHandle & handle);
    inline const void* handle() const { return data; }
    void assertExists();
};

#define NSOBJECT_OBJC_BRIDGE(t,o)((__bridge t)o) 
#define NSOBJECT_CPP_BRIDGE (__bridge void *)

class GEMetalBuffer : public GEBuffer {
public:
    NSSmartPtr metalBuffer;
    NSSmartPtr layoutDesc;
    size_t size();
    GEMetalBuffer(NSSmartPtr & buffer,NSSmartPtr & layoutDesc);
    ~GEMetalBuffer();
};

class GEMetalFence : public GEFence {
public:
    NSSmartPtr metalEvent;
    GEMetalFence(NSSmartPtr & event);
};

struct GEMetalSamplerState : public GESamplerState {
    NSSmartPtr samplerState;
    GEMetalSamplerState(NSSmartPtr & samplerState);
};

#endif

SharedHandle<OmegaGraphicsEngine> CreateMetalEngine(SharedHandle<GTEDevice> & device);
_NAMESPACE_END_

#endif


