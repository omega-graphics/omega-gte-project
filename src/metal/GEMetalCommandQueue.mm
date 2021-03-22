#import "GEMetalCommandQueue.h"

_NAMESPACE_BEGIN_
    GEMetalCommandBuffer::GEMetalCommandBuffer(){

    };
    
    void GEMetalCommandBuffer::commitToQueue(){

    };

    GEMetalCommandQueue::GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size):GECommandQueue(size),commandQueue(queue){

    };

    void GEMetalCommandQueue::present(){

    };

    SharedHandle<GECommandBuffer> GEMetalCommandQueue::getAvailableBuffer(){
        return std::make_shared<GEMetalCommandBuffer>();
    };
_NAMESPACE_END_