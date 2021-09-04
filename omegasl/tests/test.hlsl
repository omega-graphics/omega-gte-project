
struct Test {
    float2 a;
};

StructuredBuffer<Test> input : register(t0);
RWStructuredBuffer<Test> output : register(u0);

[numthreads(12,32,1)]
void CSMain(uint gid : SV_GroupThreadID){
    output[gid] = input[gid];
}