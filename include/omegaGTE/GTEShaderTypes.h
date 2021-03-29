
#ifdef TARGET_DIRECTX
#include <DirectXMath.h>
#endif

#ifdef TARGET_METAL
#include <simd/simd.h>
#endif


#ifndef OMEGAGTE_GTESHADERTYPES_H
#define OMEGAGTE_GTESHADERTYPES_H

struct OmegaGTEVertex;
struct OmegaGTEColorVertex;
struct OmegaGTETexturedVertex;


#ifdef TARGET_DIRECTX

#endif

#ifdef TARGET_METAL
struct OmegaGTEVertex {
    simd_float3 pos;
};

struct OmegaGTEColorVertex : OmegaGTEVertex {
    simd_float4 color;
};

struct OmegaGTETexturedVertex : OmegaGTEVertex {
    simd_float2 texturePos;
};
#endif


#endif