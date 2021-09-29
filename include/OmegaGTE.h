#include "omegaGTE/GTEBase.h"

#include "omegaGTE/GE.h"
#include "omegaGTE/GECommandQueue.h"
#include "omegaGTE/GEPipeline.h"
#include "omegaGTE/GERenderTarget.h"
#include "omegaGTE/GETexture.h"

#include "omegaGTE/TE.h"


#ifndef _OMEGAGTE_H
#define _OMEGAGTE_H

_NAMESPACE_BEGIN_

struct GTEDeviceFeatures {
    bool raytracing;
};

struct GTEDevice {
    typedef enum : int {
        Integrated,
        Discrete
    } Type;
    const Type type;
    const OmegaCommon::String name;
    const GTEDeviceFeatures features;
protected:
    GTEDevice(Type type,const char *name,GTEDeviceFeatures & features):type(type),name(name),features(features){

    };
};

OMEGAGTE_EXPORT OmegaCommon::Vector<SharedHandle<GTEDevice>> enumerateDevices();

struct OMEGAGTE_EXPORT GTE {
    SharedHandle<OmegaGraphicsEngine> graphicsEngine;
    SharedHandle<OmegaTessalationEngine> tessalationEngine;
};


OMEGAGTE_EXPORT GTE Init(SharedHandle<GTEDevice> & device);

OMEGAGTE_EXPORT GTE InitWithDefaultDevice();

OMEGAGTE_EXPORT void Close(GTE &gte);

_NAMESPACE_END_


#endif