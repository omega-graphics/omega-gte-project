#import <Metal/Metal.h>
#include "omegaGE/GECommandQueue.h"

#ifndef OMEGAGRAPHICSENGINE_METAL_GEMETALCOMMANDQUEUE_H
#define OMEGAGRAPHICSENGINE_METAL_GEMETALCOMMANDQUEUE_H

namespace OmegaGE {
    class GEMetalCommandBuffer : public GECommandBuffer {
        id<MTLCommandBuffer> buffer;
    public:
        
    };

    class GEMetalCommandQueue : public GECommandQueue {
        id<MTLCommandQueue> commandQueue;
    public:
        SharedHandle<GECommandBuffer> getAvailableBuffer();
        GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size);
    };
};

#endif