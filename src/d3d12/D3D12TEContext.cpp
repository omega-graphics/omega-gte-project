#include "GED3D12RenderTarget.h"
#include "omegaGTE/TE.h"
// #include "omegaGTE/GTEShaderTypes.h"

_NAMESPACE_BEGIN_

// OmegaGTETexturedVertex *convertVertex(OmegaGTE::GETexturedVertex & vertex){
//     auto rc = new OmegaGTETexturedVertex();
//     rc->pos = DirectX::XMFLOAT3(vertex.pos.getI(),vertex.pos.getJ(),vertex.pos.getK());
//     rc->texturePos = DirectX::XMFLOAT2(vertex.textureCoord.getI(),vertex.textureCoord.getJ());
//     return rc;
// };
// OmegaGTEColorVertex *convertVertex(OmegaGTE::GEColoredVertex & vertex){
//     auto rc = new OmegaGTEColorVertex();
//     rc->pos = DirectX::XMFLOAT3(vertex.pos.getI(),vertex.pos.getJ(),vertex.pos.getK());
//     rc->color = DirectX::XMFLOAT4(vertex.color.valueAt(1,1),vertex.color.valueAt(1,2),vertex.color.valueAt(1,3),vertex.color.valueAt(1,4));
//     return rc;
// };

class D3D12NativeRenderTargetTEContext : public OmegaTessalationEngineContext {
    SharedHandle<GED3D12NativeRenderTarget> target;
    public:
    void translateCoords(float x, float y, float z,GEViewport *viewport, float *x_result, float *y_result, float *z_result){
        if(viewport != nullptr){
            translateCoordsDefaultImpl(x,y,z,viewport,x_result,y_result,z_result);
        }
        else {
            
        };
    };
    // std::future<TETessalationResult> tessalateAsync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams &params, GEViewport * viewport){
        return {};
    };
    // TETessalationResult tessalateSync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    D3D12NativeRenderTargetTEContext(GED3D12NativeRenderTarget * target):target(target){

    };
};

class D3D12TextureRenderTargetTEContext : public OmegaTessalationEngineContext {
    SharedHandle<GED3D12TextureRenderTarget> target;
    public:
    void translateCoords(float x, float y, float z, GEViewport *viewport, float *x_result, float *y_result, float *z_result){
        if(viewport != nullptr){
            translateCoordsDefaultImpl(x,y,z,viewport,x_result,y_result,z_result);
        }
        else {

        };
    };
    // std::future<TETessalationResult> tessalateAsync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams &params, GEViewport * viewport){
        return {};
    };
    // TETessalationResult tessalateSync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    D3D12TextureRenderTargetTEContext(GED3D12TextureRenderTarget * target):target(target){};
};

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> &renderTarget){
    return std::make_shared<D3D12NativeRenderTargetTEContext>((GED3D12NativeRenderTarget *)renderTarget.get());
};

SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> &renderTarget){
    return std::make_shared<D3D12TextureRenderTargetTEContext>((GED3D12TextureRenderTarget *)renderTarget.get());
};



_NAMESPACE_END_