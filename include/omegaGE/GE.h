#include "GEBase.h"
#include "GECommandQueue.h"

#ifndef OMEGAGRAPHICSENGINE_GE_H
#define OMEGAGRAPHICSENGINE_GE_H

namespace OmegaGE {
    class OmegaGraphicsEngine {
    public:
        virtual SharedHandle<GECommandQueue> makeCommandQueue(unsigned maxBufferCount) = 0;
        static SharedHandle<OmegaGraphicsEngine> Create();
    };
};

#endif