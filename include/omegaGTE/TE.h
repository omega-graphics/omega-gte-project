#include "GTEBase.h"
#include <thread>
#include <future>
#include <optional> 
#include "GE.h"


#ifndef OMEGAGTE_TE_H
#define OMEGAGTE_TE_H

_NAMESPACE_BEGIN_
/**
 Defines the arguments for the tessalations operations.
*/
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
    friend class OmegaTessalationEngineContext;
public:
    /**
      Tessalate a GRect
      @param[in] rect 
      @returns TETessalationParams
    */
    static TETessalationParams Rect(GRect & rect);

    /**
      Tessalate a GRoundedRect
      @param[in] roundedRect 
      @returns TETessalationParams
    */
    static TETessalationParams RoundedRect(GRoundedRect & roundedRect);

    /**
      Tessalate a GRectangularPrism
      @param[in] rectPrism 
      @returns TETessalationParams
    */
    static TETessalationParams RectangularPrism(GRectangularPrism &rectPrism);

    /**
      Tessalate a GPyramid
      @param[in] pyramid 
      @returns TETessalationParams
    */
    static TETessalationParams Pyramid(GPyramid &pyramid);

    /**
      Tessalate a GRect
      @param[in] rect 
      @returns TETessalationParams
    */
    static TETessalationParams Ellipsoid(GEllipsoid & ellipsoid);

    /**
      Tessalate a GCylinder
      @param[in] cylinder 
      @returns TETessalationParams
    */
    static TETessalationParams Cylinder(GCylinder &cylinder);

    /**
      Tessalate a GCone
      @param[in] cone 
      @returns TETessalationParams
    */
    static TETessalationParams Cone(GCone &cone);

    /**
      Tessalate 2D vector paths
      @param[in] vectorPaths A small array with *only* 2 GVectorPath2D objects.
      @returns TETessalationParams
    */
    static TETessalationParams GraphicsPath2D(GVectorPath2D * const vectorPaths);

    /**
      Tessalate 3D vector paths
      @param[in] vectorPathCount The number of vectorPathes to tessalate 
      @param[in] vectorPaths An array of GVectorPath3D objects. (Ensure that it has the same length as the `vectorPathCount`)
      @returns TETessalationParams
    */
    static TETessalationParams GraphicsPath3D(unsigned vectorPathCount,GVectorPath3D * const vectorPaths);
};

/**
 A small struct for holding one (or more) 
 meshes that result from a tesslation operation.
*/
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
/**
 
*/
class OMEGAGTE_EXPORT OmegaTessalationEngineContext {
    friend class OmegaTessalationEngine;
protected:
    GEViewport defaultViewport;
    std::vector<std::thread *> activeThreads;

    void translateCoordsDefaultImpl(float x, float y,float z,GEViewport * viewport, float *x_result, float *y_result,float *z_result);
    virtual void translateCoords(float x, float y,float z,GEViewport * viewport, float *x_result, float *y_result,float *z_result) = 0;
    inline TETessalationResult _tessalatePriv(const TETessalationParams & params,GEViewport * viewport);
public:
    ~OmegaTessalationEngineContext();
    /**
     Tessalate according to the parameters and viewport.
     @param params 
     @param viewport
     @returns TETessalationResult
    */
    TETessalationResult tessalateSync(const TETessalationParams & params,GEViewport * viewport = nullptr);

    /**
      Performs tessalation like `tessalateSync` (@see tessalateSync), 
      however it performs the computation in a compute pipeline.
     @param params 
     @param viewport
     @returns std::future<TETessalationResult>
    */
    virtual std::future<TETessalationResult> tessalateOnGPU(const TETessalationParams & params,GEViewport * viewport = nullptr) = 0;

    /**
      Performs tessalation like `tessalateSync` (@see tessalateSync), 
      however it performs the computation in a seperate thread
     @param params 
     @param viewport
     @returns std::future<TETessalationResult>
    */
    std::future<TETessalationResult> tessalateAsync(const TETessalationParams & params,GEViewport * viewport = nullptr);

};

/**
 @brief The Omega Tessalation Engine
*/
class OMEGAGTE_EXPORT OmegaTessalationEngine {
public:
  /**
   NEVER CALL THIS FUNCTION! Please invoke GTE::Init()
  */
  static SharedHandle<OmegaTessalationEngine> Create();
    /**
        Create a Tessalation Context from a GENativeRenderTarget
        @param[in] renderTarget
        @returns SharedHandle<OmegaTessalationEngineContext>
    */
    SharedHandle<OmegaTessalationEngineContext> createTEContextFromNativeRenderTarget(SharedHandle<GENativeRenderTarget> & renderTarget);

    /**
        Create a Tessalation Context from a GETextureRenderTarget
        @param[in] renderTarget
        @returns SharedHandle<OmegaTessalationEngineContext>
    */
    SharedHandle<OmegaTessalationEngineContext> createTEContextFromTextureRenderTarget(SharedHandle<GETextureRenderTarget> & renderTarget);
};

_NAMESPACE_END_

#endif
