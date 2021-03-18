#import "GEMetal.h"
#import "GEMetalCommandQueue.h"

#import <Metal/Metal.h>

namespace OmegaGE {

    class GEMetalEngine : public OmegaGraphicsEngine {
        id<MTLDevice> metalDevice;
    public:
        SharedHandle<GECommandQueue> makeCommandQueue(unsigned int maxBufferCount){
            
        };
    };


    SharedHandle<OmegaGraphicsEngine> CreateMetalEngine(){
        return std::make_shared<GEMetalEngine>();
    };
};