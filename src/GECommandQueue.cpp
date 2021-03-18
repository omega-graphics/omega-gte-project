#include "omegaGE/GECommandQueue.h"

namespace OmegaGE {
    GECommandQueue::GECommandQueue(unsigned size):size(size){};

    unsigned GECommandQueue::getSize(){
        return size;
    };
};