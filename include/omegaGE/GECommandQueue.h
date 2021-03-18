#include "GEBase.h"

#ifndef OMEGAGRAPHICSENGINE_GECOMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_GECOMMANDQUEUE_H

namespace OmegaGE {
    class GECommandBuffer {
        
    };
    class GECommandQueue {
        unsigned size;
    protected:
        GECommandQueue(unsigned size);
    public:
        virtual SharedHandle<GECommandBuffer> getAvailableBuffer() = 0;
        virtual unsigned getSize() = 0;
    };
};

#endif