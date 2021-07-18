
#ifdef TARGET_DIRECTX
#include <DirectXMath.h>
#endif

#ifdef TARGET_METAL
#include <simd/simd.h>
#endif

#ifdef TARGET_VULKAN
#include <glm/glm.hpp>
#endif


#ifndef OMEGAGTE_GTESHADERTYPES_H
#define OMEGAGTE_GTESHADERTYPES_H

struct OmegaGTEVertex;
struct OmegaGTEColorVertex;
struct OmegaGTETexturedVertex;


#ifdef TARGET_DIRECTX

struct OmegaGTEVertex {
    DirectX::XMFLOAT3 pos;
};

struct OmegaGTEColorVertex : OmegaGTEVertex {
     DirectX::XMFLOAT4 color;
};

struct OmegaGTETexturedVertex : OmegaGTEVertex {
    DirectX::XMFLOAT2 texturePos;
};

#endif

#ifdef TARGET_METAL
struct OmegaGTEVertex {
    simd_float3 pos;
};

struct OmegaGTEColorVertex : public OmegaGTEVertex {
    simd_float4 color;
};

struct OmegaGTETexturedVertex : public OmegaGTEVertex {
    simd_float2 texturePos;
};

#endif

#ifdef TARGET_VULKAN

struct OmegaGTEVertex {
   glm::fvec3 pos;
};

struct OmegaGTEColorVertex : public OmegaGTEVertex {
    glm::fvec4 color;
};

struct OmegaGTETexturedVertex : public OmegaGTEVertex {
    glm::fvec2 texturePos;
};

#endif


#endif