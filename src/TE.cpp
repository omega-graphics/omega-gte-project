#include "omegaGTE/TE.h"
#include <thread>

_NAMESPACE_BEGIN_


struct TETessalationParams::GraphicsPath2DParams {
    GVectorPath2D *const pathes;
    unsigned pathCount;
    GraphicsPath2DParams(GVectorPath2D *const pathes,unsigned pathCount):pathes(pathes),pathCount(pathCount){};
};

struct TETessalationParams::GraphicsPath3DParams {
    GVectorPath3D *const pathes;
    unsigned pathCount;
    GraphicsPath3DParams(GVectorPath3D *const pathes,unsigned pathCount):pathes(pathes),pathCount(pathCount){};
};

TETessalationParams TETessalationParams::Rect(GRect &rect){
    TETessalationParams params;
    params.params = &rect;
    params.type = TESSALATE_RECT;
    return params;
};

TETessalationParams TETessalationParams::RoundedRect(GRoundedRect &roundedRect){
    TETessalationParams params;
    params.params = &roundedRect;
    params.type = TESSALATE_ROUNDEDRECT;
    return params;
};

TETessalationParams TETessalationParams::RectangularPrism(GRectangularPrism &prism){
    TETessalationParams params;
    params.params = &prism;
    params.type = TESSALATE_RECTANGULAR_PRISM;
    return params;
};

TETessalationParams TETessalationParams::Pyramid(GPyramid &pyramid){
    TETessalationParams params;
    params.params = &pyramid;
    params.type = TESSALATE_PYRAMID;
    return params;
};

TETessalationParams TETessalationParams::Ellipsoid(GEllipsoid &ellipsoid){
    TETessalationParams params;
    params.params = &ellipsoid;
    params.type = TESSALATE_ELLIPSOID;
    return params;
};

TETessalationParams TETessalationParams::Cylinder(GCylinder &cylinder){
    TETessalationParams params;
    params.params = &cylinder;
    params.type = TESSALATE_CYLINDER;
    return params;
};

TETessalationParams TETessalationParams::Cone(GCone &cone){
    TETessalationParams params;
    params.params = &cone;
    params.type = TESSALATE_CONE;
    return params;
};

TETessalationParams TETessalationParams::GraphicsPath2D(GVectorPath2D *const vectorPaths){
    TETessalationParams params;
    GraphicsPath2DParams * _params = new GraphicsPath2DParams(vectorPaths,2);
    params.params = _params;
    params.type = TESSALATE_GRAPHICSPATH2D;
    return params;
};

TETessalationParams TETessalationParams::GraphicsPath3D(unsigned int vectorPathCount, GVectorPath3D *const vectorPaths){
    TETessalationParams params;
    GraphicsPath3DParams * _params = new GraphicsPath3DParams(vectorPaths,vectorPathCount);
    params.params = _params;
    params.type = TESSALATE_GRAPHICSPATH3D;
    return params;
};


SharedHandle<OmegaTessalationEngine> OmegaTessalationEngine::Create(){
    return std::make_shared<OmegaTessalationEngine>();
};

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> & renderTarget);
SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> & renderTarget);

SharedHandle<OmegaTessalationEngineContext> OmegaTessalationEngine::createTEContextFromNativeRenderTarget(SharedHandle<GENativeRenderTarget> & renderTarget){
    return CreateNativeRenderTargetTEContext(renderTarget);
};

SharedHandle<OmegaTessalationEngineContext> OmegaTessalationEngine::createTEContextFromTextureRenderTarget(SharedHandle<GETextureRenderTarget> & renderTarget){
    return CreateTextureRenderTargetTEContext(renderTarget);
};



_NAMESPACE_END_
