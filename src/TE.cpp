#include "omegaGTE/TE.h"
// #include "omegaGTE/GTEShaderTypes.h"
#include <optional>
#include <thread>
#include <iostream>

_NAMESPACE_BEGIN_


struct TETessellationParams::GraphicsPath2DParams {
    GVectorPath2D *const pathes;
    unsigned pathCount;
    GraphicsPath2DParams(GVectorPath2D *const pathes,unsigned pathCount):pathes(pathes),pathCount(pathCount){};
};

struct TETessellationParams::GraphicsPath3DParams {
    GVectorPath3D *const pathes;
    unsigned pathCount;
    GraphicsPath3DParams(GVectorPath3D *const pathes,unsigned pathCount):pathes(pathes),pathCount(pathCount){};
};

TETessellationParams::Attachment TETessellationParams::Attachment::makeColor(const FVec<4> &color) {
    return {TypeColor,{color},{GPoint2D {0,0}},{GPoint3D {0,0,0}}};
}

TETessellationParams::Attachment TETessellationParams::Attachment::makeTexture2D(const GVectorPath2D &uv_map) {
    return {TypeTexture2D, {FVec<4>::Create()},{uv_map},{GPoint3D{0,0,0}}};
}

TETessellationParams::Attachment TETessellationParams::Attachment::makeTexture3D(const GVectorPath3D &uvw_map) {
    return {TypeTexture3D,{FVec<4>::Create()},{GPoint2D{0,0}},{uvw_map}};
}

void TETessellationParams::addAttachment(const Attachment &attachment) {
    attachments.push_back(attachment);
}

TETessellationParams TETessellationParams::Rect(GRect &rect){
    TETessellationParams params;
    params.params = &rect;
    params.type = TESSALATE_RECT;
    return params;
};

TETessellationParams TETessellationParams::RoundedRect(GRoundedRect &roundedRect){
    TETessellationParams params;
    params.params = &roundedRect;
    params.type = TESSALATE_ROUNDEDRECT;
    return params;
};

TETessellationParams TETessellationParams::RectangularPrism(GRectangularPrism &prism){
    TETessellationParams params;
    params.params = &prism;
    params.type = TESSALATE_RECTANGULAR_PRISM;
    return params;
};

TETessellationParams TETessellationParams::Pyramid(GPyramid &pyramid){
    TETessellationParams params;
    params.params = &pyramid;
    params.type = TESSALATE_PYRAMID;
    return params;
};

TETessellationParams TETessellationParams::Ellipsoid(GEllipsoid &ellipsoid){
    TETessellationParams params;
    params.params = &ellipsoid;
    params.type = TESSALATE_ELLIPSOID;
    return params;
};

TETessellationParams TETessellationParams::Cylinder(GCylinder &cylinder){
    TETessellationParams params;
    params.params = &cylinder;
    params.type = TESSALATE_CYLINDER;
    return params;
};

TETessellationParams TETessellationParams::Cone(GCone &cone){
    TETessellationParams params;
    params.params = &cone;
    params.type = TESSALATE_CONE;
    return params;
};

TETessellationParams TETessellationParams::GraphicsPath2D(GVectorPath2D *const vectorPaths){
    TETessellationParams params;
    GraphicsPath2DParams * _params = new GraphicsPath2DParams(vectorPaths,2);
    params.params = _params;
    params.type = TESSALATE_GRAPHICSPATH2D;
    return params;
};

TETessellationParams TETessellationParams::GraphicsPath3D(unsigned int vectorPathCount, GVectorPath3D *const vectorPaths){
    TETessellationParams params;
    GraphicsPath3DParams * _params = new GraphicsPath3DParams(vectorPaths,vectorPathCount);
    params.params = _params;
    params.type = TESSALATE_GRAPHICSPATH3D;
    return params;
};


SharedHandle<OmegaTessellationEngine> OmegaTessellationEngine::Create(){
    return std::make_shared<OmegaTessellationEngine>();
};

void OmegaTessellationEngineContext::translateCoordsDefaultImpl(float x, float y, float z, GEViewport * viewport, float *x_result, float *y_result, float *z_result){
    *x_result = x / viewport->width;
    *y_result = y / viewport->height;
    if(z_result != nullptr){
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
};

inline TETessellationResult OmegaTessellationEngineContext::_tessalatePriv(const TETessellationParams &params, GEViewport * viewport){
    assert(params.attachments.size() <= 1 && "Only 1 attachment is allowed for each tessellation params");
    TETessellationResult result;

    switch(params.type){
        case TETessellationParams::TESSALATE_RECT : {
            std::cout << "Tessalate GRect" << std::endl;
            GRect *object = (GRect *)params.params;

            TETessellationResult::TEMesh mesh {};
            TETessellationResult::TEMesh::Triangle tri {};
            float x0,x1,y0,y1;
            translateCoords(object->pos.x,object->pos.y,0.f,viewport,&x0,&y0,nullptr);
            translateCoords(object->pos.x + object->w,object->pos.y + object->h,0.f,viewport,&x1,&y1,nullptr);

            std::cout << "X0:" << x0 << ", X1:" << x1 << ", Y0:" << y0 << ", Y1:" << y1 << std::endl;

            tri.a.pt.x = x0;
            tri.a.pt.y = y0;
            tri.b.pt.x = x0;
            tri.b.pt.y = y1;
            tri.c.pt.x = x1;
            tri.c.pt.y = y0;

            std::optional<TETessellationResult::AttachmentData> extra;

            if(!params.attachments.empty()){
                auto & attachment = params.attachments.front();
                if(attachment.type == TETessellationParams::Attachment::TypeColor){
                    extra = std::make_optional<TETessellationResult::AttachmentData>({attachment.colorData.color,FVec<2>::Create(),FVec<3>::Create()});
                }
            }
            tri.a.attachment = tri.b.attachment = tri.c.attachment = extra;

            mesh.vertexTriangles.push_back(tri);

            tri.a.pt.x = x1;
            tri.a.pt.y = y1;

            mesh.vertexTriangles.push_back(tri);

            
            result.meshes.push_back(mesh);

            break;
        }
        case TETessellationParams::TESSALATE_RECTANGULAR_PRISM : {

            break;
        }
        default:
            break;
    }
    return result;
};

SharedHandle<OmegaTessellationEngineContext> CreateNativeRenderTargetTEContext(SharedHandle<GENativeRenderTarget> & renderTarget);
SharedHandle<OmegaTessellationEngineContext> CreateTextureRenderTargetTEContext(SharedHandle<GETextureRenderTarget> & renderTarget);

SharedHandle<OmegaTessellationEngineContext> OmegaTessellationEngine::createTEContextFromNativeRenderTarget(SharedHandle<GENativeRenderTarget> & renderTarget){
    return CreateNativeRenderTargetTEContext(renderTarget);
};

SharedHandle<OmegaTessellationEngineContext> OmegaTessellationEngine::createTEContextFromTextureRenderTarget(SharedHandle<GETextureRenderTarget> & renderTarget){
    return CreateTextureRenderTargetTEContext(renderTarget);
};

std::future<TETessellationResult> OmegaTessellationEngineContext::tessalateAsync(const TETessellationParams &params, GEViewport * viewport){
    std::promise<TETessellationResult> prom;
    auto fut = prom.get_future();
    activeThreads.emplace_back(new std::thread([&](std::promise<TETessellationResult> promise, size_t idx){
        promise.set_value_at_thread_exit(tessalateSync(params,viewport));
        activeThreads.erase(activeThreads.begin() + idx);
    },std::move(prom),activeThreads.size()));
    return fut;
};

TETessellationResult OmegaTessellationEngineContext::tessalateSync(const TETessellationParams &params, GEViewport * viewport){
    return _tessalatePriv(params,viewport);
};

/// @name Private API Calls
/// @{
// OmegaGTETexturedVertex *convertVertex(OmegaGTE::GETexturedVertex & vertex);
// OmegaGTEColorVertex *convertVertex(OmegaGTE::GEColoredVertex & vertex);
/// @}



// SharedHandle<GEBuffer> OmegaTessellationEngineContext::convertToVertexBuffer(SharedHandle<OmegaGraphicsEngine> & graphicsEngine,TETessellationResult & result){
//     return nullptr;
// };

// SharedHandle<GEBuffer> OmegaTessellationEngineContext::convertToVertexBuffer(SharedHandle<OmegaGraphicsEngine> & graphicsEngine,ColoredVertexVector & vertexVector){
//     std::vector<OmegaGTEColorVertex> vertices;
//     for(auto & v : vertexVector){
//         vertices.push_back(*convertVertex(v));
//     };
//     BufferDescriptor bufferDesc;
//     bufferDesc.opts = StorageOpts::Shared;
//     bufferDesc.len = sizeof(OmegaGTEColorVertex) * vertices.size();
//     auto buffer = graphicsEngine->makeBuffer(bufferDesc);
//
//     memcpy(buffer->data(),(const void *)vertices.data(),buffer->size());
//     return buffer;
// };
//
// SharedHandle<GEBuffer> OmegaTessellationEngineContext::convertToVertexBuffer(SharedHandle<OmegaGraphicsEngine> & graphicsEngine,TexturedVertexVector & vertexVector){
//     std::vector<OmegaGTETexturedVertex> vertices;
//     for(auto & v : vertexVector){
//         vertices.push_back(*convertVertex(v));
//     };
//     BufferDescriptor bufferDesc;
//     bufferDesc.opts = StorageOpts::Shared;
//     bufferDesc.len = sizeof(OmegaGTETexturedVertex) * vertices.size();
//     auto buffer = graphicsEngine->makeBuffer(bufferDesc);
//
//     memcpy(buffer->data(),(const void *)vertices.data(),buffer->size());
//     return buffer;
// };

OmegaTessellationEngineContext::~OmegaTessellationEngineContext(){
    for(auto t : activeThreads){
        if(t->joinable()){
            t->join();
        }
    };
};



_NAMESPACE_END_
