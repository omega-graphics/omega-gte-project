#include "OmegaGTE.h"

_NAMESPACE_BEGIN_

GTE Init(void *device){
    auto ge = OmegaGraphicsEngine::Create(device);
    return {ge,OmegaTessalationEngine::Create()};
};


void Close(GTE &gte){
    gte.graphicsEngine.reset();
    gte.tessalationEngine.reset();
};

_NAMESPACE_END_