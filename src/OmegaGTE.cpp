#include "OmegaGTE.h"

_NAMESPACE_BEGIN_

GTE Init(){
    auto ge = OmegaGraphicsEngine::Create();
    return {ge,OmegaTessalationEngine::Create(),std::make_shared<GTEShaderBuilder>(ge.get())};
};


void Close(GTE &gte){
    gte.graphicsEngine.reset();
    gte.tessalationEngine.reset();
};

_NAMESPACE_END_