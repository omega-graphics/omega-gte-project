#include "omegaGTE/GECommandQueue.h"

_NAMESPACE_BEGIN_
    GERenderPassDescriptor::ColorAttachment::ClearColor::ClearColor(float r,float g,float b,float a):r(r),g(g),b(b),a(a){

    };

    GECommandQueue::GECommandQueue(unsigned size):size(size){};

    unsigned GECommandQueue::getSize(){
        return size;
    };
_NAMESPACE_END_
