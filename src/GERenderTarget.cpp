#include "omegaGTE/GERenderTarget.h"
#include "omegaGTE/GTEBase.h"
_NAMESPACE_BEGIN_
   #ifdef TARGET_DIRECTX
   SharedHandle<GENativeRenderTarget> GENativeRenderTarget::Create(HWND hwnd){
       
   };
   #endif

   #if defined(TARGET_METAL) && defined(__OBJC__)
   SharedHandle<GENativeRenderTarget> GENativeRenderTarget::Create(CAMetalLayer * metalLayer){
       
   };
   #endif
_NAMESPACE_END_