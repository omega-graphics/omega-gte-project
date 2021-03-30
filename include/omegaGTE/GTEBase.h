#include <memory>
#include <vector>

#ifndef OMEGAGTE_GTEBASE_H
#define OMEGAGTE_GTEBASE_H

#ifdef TARGET_DIRECTX
#define OMEGAGTE_EXPORT __declspec( dllexport ) 
#endif

#define _NAMESPACE_BEGIN_ namespace OmegaGTE {
#define _NAMESPACE_END_ }

_NAMESPACE_BEGIN_
    struct GRect {
        float x,y,w,h;
    };
    template<class _Ty,size_t rows,size_t columns>
    class Matrix {
        std::array<std::array<_Ty,columns>,rows> data;
    public:
        static Matrix Identity();
//        static Matrix Create(std::initializer_list<std::initializer_list<_Ty>> data);
    };


    template<class _Ty>
    using UniqueHandle = std::unique_ptr<_Ty>;

    template<class _Ty>
    using SharedHandle = std::shared_ptr<_Ty>;

_NAMESPACE_END_

#endif
