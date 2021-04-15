#include "GTEBase.h"
#include <thread>
#include <future>
#include <optional> 
#include "GE.h"


#ifndef OMEGAGTE_TE_H
#define OMEGAGTE_TE_H

_NAMESPACE_BEGIN_

struct OMEGAGTE_EXPORT TETessalationParams {
    private:
    struct GraphicsPath2DParams;
    struct GraphicsPath3DParams;
    typedef enum : unsigned char {
        TESSALATE_RECT,
        TESSALATE_ROUNDEDRECT,
        TESSALATE_RECTANGULAR_PRISM,
        TESSALATE_PYRAMID,
        TESSALATE_ELLIPSOID,
        TESSALATE_CYLINDER,
        TESSALATE_CONE,
        TESSALATE_GRAPHICSPATH2D,
        TESSALATE_GRAPHICSPATH3D
    } TessalationType;
    TessalationType type;
    void * params;
public:
    static TETessalationParams Rect(GRect & rect);
    static TETessalationParams RoundedRect(GRoundedRect & roundedRect);
    static TETessalationParams RectangularPrism(GRectangularPrism &rectPrism);
    static TETessalationParams Pyramid(GPyramid &pyramid);
    static TETessalationParams Ellipsoid(GEllipsoid & ellipsoid);
    static TETessalationParams Cylinder(GCylinder &cylinder);
    static TETessalationParams Cone(GCone &cone);
    static TETessalationParams GraphicsPath2D(GVectorPath2D * const vectorPaths);
    static TETessalationParams GraphicsPath3D(unsigned vectorPathCount,GVectorPath3D * const vectorPaths);
};

struct OMEGAGTE_EXPORT TETessalationResult {
    struct OMEGAGTE_EXPORT TEMesh {
        struct OMEGAGTE_EXPORT Triangle {
            GPoint3D a;
            GPoint3D b;
            GPoint3D c;
        };
        std::vector<Triangle> vertexTriangles;
    };
    std::vector<TEMesh> meshes;
};

class OMEGAGTE_EXPORT OmegaTessalationEngineContext {
    friend class OmegaTessalationEngine;
public:
    virtual TETessalationResult tessalateSync(const TETessalationParams & params,std::optional<GEViewport> viewport = {}) = 0;
    virtual std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams & params,std::optional<GEViewport> viewport = {}) = 0;
    virtual  std::future<TETessalationResult> tessalateAsync(const TETessalationParams & params,std::optional<GEViewport> viewport = {}) = 0;
};


class OMEGAGTE_EXPORT OmegaTessalationEngine {
    static SharedHandle<OmegaTessalationEngine> Create();
    IN_INIT_SCOPE
public:
    SharedHandle<OmegaTessalationEngineContext> createTEContextFromNativeRenderTarget(SharedHandle<GENativeRenderTarget> & renderTarget);
    SharedHandle<OmegaTessalationEngineContext> createTEContextFromTextureRenderTarget(SharedHandle<GETextureRenderTarget> & renderTarget);
};

_NAMESPACE_END_

#endif
