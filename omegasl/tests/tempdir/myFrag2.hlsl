struct VertexRaster2{
    float4 pos:SV_Position;
    float2 coord:TEXCOORD;
};

Texture2D myTexture: register(t0);
SamplerState mySampler{
    Filter=MIN_MAG_MIP_LINEAR
    AddressU=Wrap
    AddressV=Wrap
    AddressW=Wrap
    MaxAnisotropy=16
};
float4 myFrag2(VertexRaster2 raster) : SV_TARGET{
  return myTexture.Sample(mySampler,raster.coord);
}
