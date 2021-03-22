#import <Metal/Metal.h>
#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGRAPHICSENGINE_METAL_GEMETALCOMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_METAL_GEMETALCOMMANDQUEUE_H

_NAMESPACE_BEGIN_
    class GEMetalCommandBuffer : public GECommandBuffer {
        id<MTLCommandBuffer> buffer;
    public:
        GEMetalCommandBuffer();
        void commitToQueue();
    };

    class GEMetalCommandQueue : public GECommandQueue {
        id<MTLCommandQueue> commandQueue;
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size);
        void present();
    };
_NAMESPACE_END_

#endif