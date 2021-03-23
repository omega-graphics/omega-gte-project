#import <Metal/Metal.h>
#include "omegaGTE/GECommandQueue.h"

#ifndef OMEGAGRAPHICSENGINE_METAL_GEMETALCOMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_METAL_GEMETALCOMMANDQUEUE_H

_NAMESPACE_BEGIN_

    class GEMetalCommandQueue;
    class GEMetalCommandBuffer : public GECommandBuffer {
        id<MTLCommandBuffer> buffer;
        GEMetalCommandQueue *parentQueue;
    public:
        GEMetalCommandBuffer(id<MTLCommandBuffer> buffer,GEMetalCommandQueue *parentQueue);
        void commitToQueue();
    };

    class GEMetalCommandQueue : public GECommandQueue {
        id<MTLCommandQueue> commandQueue;
        NSMutableArray<id<MTLCommandBuffer>> * commandBuffers;
        friend class GEMetalCommandBuffer;
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size);
        ~GEMetalCommandQueue();
        void present();
    };
_NAMESPACE_END_

#endif