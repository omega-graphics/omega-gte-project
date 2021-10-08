#include "OmegaGTE.h"

_NAMESPACE_BEGIN_

GTE Init(SharedHandle<GTEDevice> & device){
    auto ge = OmegaGraphicsEngine::Create(device);
    return {ge, OmegaTessellationEngine::Create()};
};

GTE InitWithDefaultDevice(){
    auto devices = enumerateDevices();
    return Init(devices.front());
}


void Close(GTE &gte){
    gte.graphicsEngine.reset();
    gte.tessalationEngine.reset();
};

_NAMESPACE_END_