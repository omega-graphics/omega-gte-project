#include "GEBase.h"

#ifndef OMEGAGRAPHICSENGINE_GECOMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_GECOMMANDQUEUE_H

namespace OmegaGE {
    class GECommandBuffer {
    public:

        virtual void commitToBuffer() = 0;
    };
    class GECommandQueue {
        unsigned size;
    protected:
        GECommandQueue(unsigned size);
    public:
        virtual SharedHandle<GECommandBuffer> getAvailableBuffer() = 0;
        unsigned getSize();
        virtual void present() = 0;
    };
};

#endif