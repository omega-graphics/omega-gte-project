#include "d3d12/GED3D12RenderTarget.h"
#include "omegaGTE/TE.h"

_NAMESPACE_BEGIN_

class D3D12NativeRenderTargetTEContext : public OmegaTessalationEngineContext {
    SharedHandle<GED3D12NativeRenderTarget> target;
    public:
    std::future<TETessalationResult> tessalateAsync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    TETessalationResult tessalateSync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    D3D12NativeRenderTargetTEContext(GED3D12NativeRenderTarget * target):target(target){};
};

class D3D12TextureRenderTargetTEContext : public OmegaTessalationEngineContext {
    SharedHandle<GED3D12TextureRenderTarget> target;
    public:
    std::future<TETessalationResult> tessalateAsync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    TETessalationResult tessalateSync(const TETessalationParams &params, std::optional<GEViewport> viewport = {}){};
    D3D12TextureRenderTargetTEContext(GED3D12TextureRenderTarget * target):target(target){};
};

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> &renderTarget){
    return std::make_shared<D3D12NativeRenderTargetTEContext>((GED3D12NativeRenderTarget *)renderTarget.get());
};

SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> &renderTarget){
    return std::make_shared<D3D12TextureRenderTargetTEContext>((GED3D12TextureRenderTarget *)renderTarget.get());
};



_NAMESPACE_END_