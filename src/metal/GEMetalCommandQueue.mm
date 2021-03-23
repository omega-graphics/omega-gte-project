#import "GEMetalCommandQueue.h"

#import <QuartzCore/QuartzCore.h>
_NAMESPACE_BEGIN_
    GEMetalCommandBuffer::GEMetalCommandBuffer(id<MTLCommandBuffer> buffer,GEMetalCommandQueue *parentQueue):buffer(buffer),parentQueue(parentQueue){

    };
    
    void GEMetalCommandBuffer::commitToQueue(){
        [buffer enqueue];
        [parentQueue->commandBuffers addObject:buffer];
    };

    GEMetalCommandQueue::GEMetalCommandQueue(id<MTLCommandQueue> queue,unsigned size):GECommandQueue(size),commandQueue(queue){
        commandBuffers = [[NSMutableArray alloc] init];
    };

    void GEMetalCommandQueue::present(){
        @autoreleasepool {
            CAMetalLayer * metalLayer;
            id<CAMetalDrawable> drawable = [metalLayer nextDrawable];

            auto lastBuffer = [commandQueue commandBuffer];
            [lastBuffer presentDrawable:drawable];
            [lastBuffer enqueue];
            [commandBuffers addObject:lastBuffer];
            for(id<MTLCommandBuffer> buffer in commandBuffers){
                [buffer commit];
                [buffer waitUntilScheduled];
            };
        };
    };

    GEMetalCommandQueue::~GEMetalCommandQueue(){
        [commandBuffers release];
    };

    SharedHandle<GECommandBuffer> GEMetalCommandQueue::getAvailableBuffer(){
        ++currentlyOccupied;
        return std::make_shared<GEMetalCommandBuffer>([commandQueue commandBuffer],this);
    };
_NAMESPACE_END_