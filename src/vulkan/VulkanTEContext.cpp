

#include "omegaGTE/GTEBase.h"
#include "omegaGTE/TE.h"


_NAMESPACE_BEGIN_

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> & renderTarget){
    return nullptr;
};

SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> & renderTarget){
    return nullptr;
};

_NAMESPACE_END_
