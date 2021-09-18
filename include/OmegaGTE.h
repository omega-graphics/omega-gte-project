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

struct OMEGAGTE_EXPORT GTE {
    SharedHandle<OmegaGraphicsEngine> graphicsEngine;
    SharedHandle<OmegaTessalationEngine> tessalationEngine;
};



OMEGAGTE_EXPORT GTE Init();
OMEGAGTE_EXPORT void Close(GTE &gte);

_NAMESPACE_END_


#endif