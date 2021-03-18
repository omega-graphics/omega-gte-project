#import "GEMetalCommandQueue.h"

namespace OmegaGE {
    GEMetalCommandQueue::GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size):GECommandQueue(size),commandQueue(queue){

    };
    SharedHandle<GECommandBuffer> GECommandQueue::getAvailableBuffer(){
        return std::make_shared<GEMetalCommandBuffer>();
    };
};