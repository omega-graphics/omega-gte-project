#include "GTEBase.h"

// #ifdef TARGET_DIRECTX
// #include <windows.h>
// #endif


#ifndef OMEGAGTE_GERENDERTARGET_H
#define OMEGAGTE_GERENDERTARGET_H

_NAMESPACE_BEGIN_

    class GERenderTarget {
        
    };
    class GENativeRenderTarget : public GERenderTarget {
        // #ifdef TARGET_DIRECTX
        // static SharedHandle<GENativeRenderTarget> Create(HWND hwnd);
        // #endif
        public:
     };
     class GETextureRenderTarget : public GERenderTarget {
         public:
     };

_NAMESPACE_END_

#endif
