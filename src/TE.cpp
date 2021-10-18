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

inline void OmegaTessellationEngineContext::_tessalatePriv(const TETessellationParams &params, GEViewport * viewport,TETessellationResult & result){
    assert(params.attachments.size() <= 1 && "Only 1 attachment is allowed for each tessellation params");

    switch(params.type){
        case TETessellationParams::TESSALATE_RECT : {
            std::cout << "Tessalate GRect" << std::endl;
            GRect *object = (GRect *)params.params;

            TETessellationResult::TEMesh mesh {TETessellationResult::TEMesh::TopologyTriangle};
            TETessellationResult::TEMesh::Polygon tri {};
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

            mesh.vertexPolygons.push_back(tri);

            tri.a.pt.x = x1;
            tri.a.pt.y = y1;

            mesh.vertexPolygons.push_back(tri);

            
            result.meshes.push_back(mesh);

            break;
        }
        case TETessellationParams::TESSALATE_ROUNDEDRECT : {
            auto object = (GRoundedRect *)params.params;

            GRect middle_rect {object->rad_x,object->rad_y,object->w - (2 * object->rad_x),object->h - (2 * object->rad_y)};

            auto middle_rect_params = TETessellationParams::Rect(middle_rect);

            _tessalatePriv(middle_rect_params,viewport,result);

            auto tessalateArc = [&](GPoint2D start,float rad_x,float rad_y,float angle_start,float angle_end,float _arcStep){
                TETessellationResult::TEMesh m {TETessellationResult::TEMesh::TopologyTriangleStrip};
                GPoint3D pt_a {start.x,start.y,0.f};
                auto _angle_it = angle_start;
                while(_angle_it <= angle_end){
                    TETessellationResult::TEMesh::Polygon p {};

                    auto x_f = std::cosf(_angle_it) * rad_x;
                    auto y_f = std::sinf(_angle_it) * rad_y;

                    x_f += start.x;
                    y_f += start.y;

                    p.a.pt = pt_a;
                    p.b.pt = GPoint3D {x_f,y_f,0.f};


                    _angle_it += _arcStep;

                    x_f = std::cosf(_angle_it) * rad_x;
                    y_f = std::sinf(_angle_it) * rad_y;

                    x_f += start.x;
                    y_f += start.y;

                    p.c.pt = GPoint3D {x_f,y_f,0.f};

                    m.vertexPolygons.push_back(p);
                }
                result.meshes.push_back(m);
            };

            /// Bottom Left Arc
            tessalateArc(GPoint2D {object->rad_x,object->rad_y},object->rad_x,object->rad_y,float(3.f * PI)/2.f,PI,-arcStep);

            /// Left Rect
            middle_rect = GRect {GPoint2D{0.f,object->rad_y},object->rad_x,object->h - (2 * object->rad_y)};
            middle_rect_params = TETessellationParams::Rect(middle_rect);

            _tessalatePriv(middle_rect_params,viewport,result);
            /// Top Left Arc
            tessalateArc(GPoint2D {object->rad_x,object->h - object->rad_y},object->rad_x,object->rad_y,PI,float(PI)/2.f,-arcStep);

            /// Top Rect
            middle_rect = GRect {GPoint2D{object->rad_x,object->h - object->rad_y},object->w - (object->rad_x * 2),object->rad_y};
            middle_rect_params = TETessellationParams::Rect(middle_rect);

            _tessalatePriv(middle_rect_params,viewport,result);
            /// Top Right Arc
            tessalateArc(GPoint2D {object->w - object->rad_x,object->h - (object->rad_y)},object->rad_x,object->rad_y,float(PI)/2.f,0,-arcStep);

            /// Right Rect
            middle_rect = GRect {GPoint2D{object->w - object->rad_x,object->rad_y},object->rad_x,object->h - (2 * object->rad_y)};
            middle_rect_params = TETessellationParams::Rect(middle_rect);

            _tessalatePriv(middle_rect_params,viewport,result);

            /// Bottom Right Arc
            tessalateArc(GPoint2D {object->w - object->rad_x,object->rad_y},object->rad_x,object->rad_y,0,-float(PI)/2.f,-arcStep);

            /// Bottom Rect
            middle_rect = GRect {GPoint2D{0.f,0.f},object->w - (object->rad_x * 2),object->rad_y};
            middle_rect_params = TETessellationParams::Rect(middle_rect);

            break;
        }
        case TETessellationParams::TESSALATE_RECTANGULAR_PRISM : {

            break;
        }
        default:
            break;
    }

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
    TETessellationResult res;
    _tessalatePriv(params,viewport,res);
    return res;
};

OmegaTessellationEngineContext::~OmegaTessellationEngineContext(){
    for(auto t : activeThreads){
        if(t->joinable()){
            t->join();
        }
    };
};

void TETessellationResult::TEMesh::translate(float x, float y, float z,const GEViewport & viewport) {
    auto _x = x/viewport.width;
    auto _y = y/viewport.height;
    auto _z = z/viewport.farDepth;

    for(auto & polygon : vertexPolygons){
        polygon.a.pt.x += _x;
        polygon.b.pt.x += _x;
        polygon.c.pt.x += _x;

        polygon.a.pt.y += _y;
        polygon.b.pt.y += _y;
        polygon.c.pt.y += _y;

        polygon.a.pt.z += _z;
        polygon.b.pt.z += _z;
        polygon.c.pt.z += _z;
    }
}

void TETessellationResult::TEMesh::rotate(float pitch, float yaw, float roll) {

    /// Pitch Rotation -- X Axis.
    /// Yaw Rotation -- Y Axis.
    /// Roll Rotation -- Z Axis.

    auto cos_pitch = std::cosf(pitch),sin_pitch = std::sinf(pitch);
    auto cos_yaw = std::cosf(yaw),sin_yaw = std::sinf(yaw);
    auto cos_roll = std::cosf(roll),sin_roll = std::sinf(roll);

    auto rotatePoint = [&](GPoint3D & pt){
        /// Pitch Rotation
        pt.x *= 1;
        pt.z = (0 + (cos_pitch * pt.z) - (sin_pitch * pt.y));
        pt.y = (0 + (sin_pitch * pt.z) + (cos_pitch * pt.y));
        /// Yaw Rotation
        pt.y *= 1;
        pt.x = (0 + (cos_yaw * pt.x) - (sin_yaw * pt.z));
        pt.z = (0 + (sin_yaw * pt.x) + (cos_yaw * pt.z));
        /// Roll Rotation
        pt.z *= 1;
        pt.x = (0 + (cos_roll * pt.x) - (sin_roll * pt.y));
        pt.y = (0 + (sin_roll * pt.x) + (cos_roll * pt.y));
    };

    for(auto & polygon : vertexPolygons){
        rotatePoint(polygon.a.pt);
        rotatePoint(polygon.b.pt);
        rotatePoint(polygon.c.pt);
    }
}

void TETessellationResult::TEMesh::scale(float w, float h,float l) {
    for(auto & polygon : vertexPolygons){
        polygon.a.pt.x *= w;
        polygon.b.pt.x *= w;
        polygon.c.pt.x *= w;

        polygon.a.pt.y *= h;
        polygon.b.pt.y *= h;
        polygon.c.pt.y *= h;

        polygon.a.pt.z *= l;
        polygon.b.pt.z *= l;
        polygon.c.pt.z *= l;
    }
}



_NAMESPACE_END_
