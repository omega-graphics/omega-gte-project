#include "GTEBase.h"

#ifndef OMEGAGRAPHICSENGINE_GECOMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_GECOMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GECommandBuffer {
    public:
        virtual void commitToQueue() = 0;
    };
    class GECommandQueue {
        unsigned size;
    protected:
        unsigned currentlyOccupied = 0;
        GECommandQueue(unsigned size);
    public:
        virtual SharedHandle<GECommandBuffer> getAvailableBuffer() = 0;
        unsigned getSize();
        virtual void present() = 0;
    };
_NAMESPACE_END_

#endif