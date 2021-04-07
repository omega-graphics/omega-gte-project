#include "GTEBase.h"
#include <thread>
#include <future>

#ifndef OMEGAGTE_TE_H
#define OMEGAGTE_TE_H

_NAMESPACE_BEGIN_

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

#endif
