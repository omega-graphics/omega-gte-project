// Warning! This file was generated by omegaslc
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexRaster {
    simd_float4 pos[[position]];
    simd_float4 color;
};


fragment simd_float4 myFrag (VertexRaster raster [[stage_in]]){

    return raster.color;
}
