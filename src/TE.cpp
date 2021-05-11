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

void OmegaTessalationEngineContext::translateCoordsDefaultImpl(float x, float y,float z,std::optional<GEViewport> & viewport, float *x_result, float *y_result,float *z_result){
    *x_result = x / viewport->width;
    *y_result = y / viewport->height;
    if(z > 0.0){
        *z_result = z / viewport->farDepth;
    }
    else if(z < 0.0){
        *z_result = z / viewport->nearDepth;
    }
    else {
        *z_result = z;
    };
};

inline TETessalationResult OmegaTessalationEngineContext::_tessalatePriv(const TETessalationParams &params,std::optional<GEViewport> viewport){
    TETessalationResult result;
    switch(params.type){
        case TETessalationParams::TESSALATE_RECT : {
            GRect *object = (GRect *)params.params;

            TETessalationResult::TEMesh mesh;
            TETessalationResult::TEMesh::Triangle tri;
            float x0,x1,y0,y1;
            translateCoords(object->pos.x,object->pos.y,0.f,viewport,&x0,&y0,nullptr);
            translateCoords(object->pos.x + object->w,object->pos.y + object->h,0.f,viewport,&x1,&y1,nullptr);


            tri.a.x = x0;
            tri.a.y = y0;
            tri.b.x = x0;
            tri.b.y = y1;
            tri.c.x = x1;
            tri.c.y = y0;

            mesh.vertexTriangles.push_back(tri);

            tri.a.x = x1;
            tri.a.y = y1;

            mesh.vertexTriangles.push_back(tri);
            
            result.meshes.push_back(mesh);

            break;
        }
        case TETessalationParams::TESSALATE_RECTANGULAR_PRISM : {

            break;
        }
        default:
            break;
    }
    return result;
};

SharedHandle<OmegaTessalationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> & renderTarget);
SharedHandle<OmegaTessalationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> & renderTarget);

SharedHandle<OmegaTessalationEngineContext> OmegaTessalationEngine::createTEContextFromNativeRenderTarget(SharedHandle<GENativeRenderTarget> & renderTarget){
    return CreateNativeRenderTargetTEContext(renderTarget);
};

SharedHandle<OmegaTessalationEngineContext> OmegaTessalationEngine::createTEContextFromTextureRenderTarget(SharedHandle<GETextureRenderTarget> & renderTarget){
    return CreateTextureRenderTargetTEContext(renderTarget);
};

std::future<TETessalationResult> OmegaTessalationEngineContext::tessalateAsync(const TETessalationParams &params,std::optional<GEViewport> viewport){
    std::promise<TETessalationResult> prom;
    auto fut = prom.get_future();
    activeThreads.emplace_back([&](std::promise<TETessalationResult> promise,size_t idx){
        promise.set_value_at_thread_exit(tessalateSync(params,viewport));
        activeThreads.erase(activeThreads.begin() + idx);
    },std::move(prom),activeThreads.size());
    return fut;
};

TETessalationResult OmegaTessalationEngineContext::tessalateSync(const TETessalationParams &params,std::optional<GEViewport> viewport){
    return _tessalatePriv(params,viewport);
};

OmegaTessalationEngineContext::~OmegaTessalationEngineContext(){
    for(auto & t : activeThreads){
        if(t.joinable()){
            t.join();
        }
    };
};



_NAMESPACE_END_
