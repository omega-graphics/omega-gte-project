#include <memory>

#ifndef OMEGAGRAPHICSENGINE_GEBASE_H
#define OMEGAGRAPHICSENGINE_GEBASE_H

namespace OmegaGE {
    template<class _Ty>
    using UniqueHandle = std::unique_ptr<_Ty>;

    template<class _Ty>
    using SharedHandle = std::shared_ptr<_Ty>;
}

#endif