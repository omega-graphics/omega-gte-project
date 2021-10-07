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
            /// Use Entire Render Target as Viewport.
            RECT rc;
            GetClientRect(target->hwnd,&rc);
            GEViewport geViewport {0,0,float(rc.right - rc.left),float(rc.bottom - rc.top)};
            translateCoordsDefaultImpl(x,y,z,&geViewport,x_result,y_result,z_result);
        };
    };
    // std::future<TETessalationResult> tessalateAsync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams &params, GEViewport * viewport){
        return {};
    };
    // TETessalationResult tessalateSync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    D3D12NativeRenderTargetTEContext(SharedHandle<GED3D12NativeRenderTarget> target):target(target){

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
    D3D12TextureRenderTargetTEContext(SharedHandle<GED3D12TextureRenderTarget> target):target(target){};
};

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> &renderTarget){
    return std::make_shared<D3D12NativeRenderTargetTEContext>(std::dynamic_pointer_cast<GED3D12NativeRenderTarget>(renderTarget));
};

SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> &renderTarget){
    return std::make_shared<D3D12TextureRenderTargetTEContext>(std::dynamic_pointer_cast<GED3D12TextureRenderTarget>(renderTarget));
};



_NAMESPACE_END_