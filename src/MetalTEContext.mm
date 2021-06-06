#include "omegaGTE/TE.h"
#include "metal/GEMetalRenderTarget.h"
#include "omegaGTE/GTEShaderTypes.h"

#import <simd/simd.h>

_NAMESPACE_BEGIN_

OmegaGTEColorVertex *convertVertex(OmegaGTE::GEColoredVertex & vertex){
    auto * v = new OmegaGTEColorVertex();
    v->color = {vertex.color.valueAt(1,1),vertex.color.valueAt(1,2),vertex.color.valueAt(1,3),vertex.color.valueAt(1,4)};
    v->pos = {vertex.pos.getI(),vertex.pos.getI(),vertex.pos.getK()};
    return v;
};

OmegaGTETexturedVertex *convertVertex(OmegaGTE::GETexturedVertex & vertex){
    auto * v = new OmegaGTETexturedVertex();
    v->texturePos = {vertex.textureCoord.getI(),vertex.textureCoord.getJ()};
    v->pos = {vertex.pos.getI(),vertex.pos.getI(),vertex.pos.getK()};
    return v;
};


class MetalNativeRenderTargetTEContext : public OmegaTessalationEngineContext {
    SharedHandle<GEMetalNativeRenderTarget> target;
    public:
    // std::future<TETessalationResult> tessalateAsync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){
    //     std::promise<TETessalationResult> prom;
    //     auto fut = prom.get_future();
    //     std::thread t([&](std::promise<TETessalationResult> promise){
    //         promise.set_value_at_thread_exit(tessalateSync(params,viewport));
    //     },std::move(prom));
    //     return fut;
    // };
    std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    // TETessalationResult tessalateSync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){
    //     return _tessalatePriv(params,viewport);
    // };
    void translateCoords(float x, float y,float z,std::optional<GEViewport> & viewport,float *x_result, float *y_result,float *z_result){
        if(viewport.has_value()){
            translateCoordsDefaultImpl(x,y,z,viewport,x_result,y_result,z_result);
        }
        else {

        };
    };
    MetalNativeRenderTargetTEContext(GEMetalNativeRenderTarget * target):target(target){};
};

class MetalTextureRenderTargetTEContext : public OmegaTessalationEngineContext {
    SharedHandle<GEMetalTextureRenderTarget> target;
    public:
    // std::future<TETessalationResult> tessalateAsync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){
        
    // };
    std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    // TETessalationResult tessalateSync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){
    //     return _tessalatePriv(params,viewport);
    // };
    void translateCoords(float x, float y,float z,std::optional<GEViewport> & viewport, float *x_result, float *y_result,float *z_result){
        if(viewport.has_value()){
            translateCoordsDefaultImpl(x,y,z,viewport,x_result,y_result,z_result);
        }
        else {

        };
    };
    MetalTextureRenderTargetTEContext(GEMetalTextureRenderTarget * target):target(target){};
};

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> &renderTarget){
    return std::make_shared<MetalNativeRenderTargetTEContext>((GEMetalNativeRenderTarget *)renderTarget.get());
};

SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> &renderTarget){
    return std::make_shared<MetalTextureRenderTargetTEContext>((GEMetalTextureRenderTarget *)renderTarget.get());
};



_NAMESPACE_END_