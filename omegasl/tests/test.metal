#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Test
{
    float2 a;
};

struct _output
{
    Test _data[1];
};

struct _input
{
    Test _data[1];
};

kernel void CSMain(device _output& _output_1 [[buffer(0)]], const device _input& _input_1 [[buffer(1)]], uint3 gl_LocalInvocationID [[thread_position_in_threadgroup]])
{
    _output_1._data[gl_LocalInvocationID.x] = _input_1._data[gl_LocalInvocationID.x];
}

