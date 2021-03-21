#include "GTEBase.h"
#include <thread>
#include <future>

_NAMESPACE_BEGIN_

class TECoordinateSpaceContext {

};

struct TETessalationParams {
    
};

struct TETessalationResult {

};


class OmegaTessalationEngine {
public:
    static SharedHandle<OmegaTessalationEngine> Create();
    TETessalationResult tessalateSync(const TETessalationParams & params);
    std::promise<TETessalationResult> tessalateAsync(const TETessalationParams & params);
};

_NAMESPACE_END_