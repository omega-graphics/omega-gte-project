#include "GTEBase.h"
#include <thread>
#include <future>
#include <optional> 
#include "GE.h"


#ifndef OMEGAGTE_TE_H
#define OMEGAGTE_TE_H

_NAMESPACE_BEGIN_
/**
 *
 Defines the arguments for the tessellation operations.
*/
struct OMEGAGTE_EXPORT TETessellationParams {
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
    friend class OmegaTessellationEngineContext;
public:
    struct Attachment {
        typedef enum : int {
            TypeColor,
            TypeTexture2D,
            TypeTexture3D
        } Type;
        Type type;
        struct {
            FVec<4> color;
        } colorData;
        struct {
            GVectorPath2D uv_map;
        } texture2DData;
        struct {
            GVectorPath3D uvw_map;
        } texture3DData;

        static Attachment makeColor(const FVec<4> & color);
        static Attachment makeTexture2D(const GVectorPath2D & uv_map);
        static Attachment makeTexture3D(const GVectorPath3D & uvw_map);
    };

    void addAttachment(const Attachment &attachment);
private:
    OmegaCommon::Vector<Attachment> attachments;
public:
    /**
      Tessalate a GRect
      @param[in] rect 
      @returns TETessalationParams
    */
    static TETessellationParams Rect(GRect & rect);

    /**
      Tessalate a GRoundedRect
      @param[in] roundedRect 
      @returns TETessalationParams
    */
    static TETessellationParams RoundedRect(GRoundedRect & roundedRect);

    /**
      Tessalate a GRectangularPrism
      @param[in] rectPrism 
      @returns TETessalationParams
    */
    static TETessellationParams RectangularPrism(GRectangularPrism &rectPrism);

    /**
      Tessalate a GPyramid
      @param[in] pyramid 
      @returns TETessalationParams
    */
    static TETessellationParams Pyramid(GPyramid &pyramid);

    /**
      Tessalate a GRect
      @param[in] rect 
      @returns TETessalationParams
    */
    static TETessellationParams Ellipsoid(GEllipsoid & ellipsoid);

    /**
      Tessalate a GCylinder
      @param[in] cylinder 
      @returns TETessalationParams
    */
    static TETessellationParams Cylinder(GCylinder &cylinder);

    /**
      Tessalate a GCone
      @param[in] cone 
      @returns TETessalationParams
    */
    static TETessellationParams Cone(GCone &cone);

    /**
      Tessalate 2D vector paths
      @param[in] vectorPaths A small array with *only* 2 GVectorPath2D objects.
      @returns TETessalationParams
    */
    static TETessellationParams GraphicsPath2D(GVectorPath2D * const vectorPaths);

    /**
      Tessalate 3D vector paths
      @param[in] vectorPathCount The number of vectorPathes to tessalate 
      @param[in] vectorPaths An array of GVectorPath3D objects. (Ensure that it has the same length as the `vectorPathCount`)
      @returns TETessalationParams
    */
    static TETessellationParams GraphicsPath3D(unsigned vectorPathCount,GVectorPath3D * const vectorPaths);
};

/**
 A small struct for holding one (or more) 
 meshes that result from a tesslation operation.
*/
struct OMEGAGTE_EXPORT TETessellationResult {
    struct OMEGAGTE_EXPORT AttachmentData {
        FVec<4> color;
        FVec<2> texture2Dcoord;
        FVec<3> texture3Dcoord;
    };
    struct OMEGAGTE_EXPORT TEMesh {
        struct OMEGAGTE_EXPORT Triangle {
            struct {
                GPoint3D pt;
                std::optional<AttachmentData> attachment;
            } a,b,c;
        };
        std::vector<Triangle> vertexTriangles;
    };
    std::vector<TEMesh> meshes;
};
/**
 
*/
class OMEGAGTE_EXPORT OmegaTessellationEngineContext {
    friend class OmegaTessellationEngine;
protected:
    GEViewport defaultViewport;
    std::vector<std::thread *> activeThreads;

    void translateCoordsDefaultImpl(float x, float y,float z,GEViewport * viewport, float *x_result, float *y_result,float *z_result);
    virtual void translateCoords(float x, float y,float z,GEViewport * viewport, float *x_result, float *y_result,float *z_result) = 0;
    inline TETessellationResult _tessalatePriv(const TETessellationParams & params, GEViewport * viewport);
public:
    ~OmegaTessellationEngineContext();
    /**
     Tessalate according to the parameters and viewport.
     @param params 
     @param viewport
     @returns TETessellationResult
    */
    TETessellationResult tessalateSync(const TETessellationParams & params, GEViewport * viewport = nullptr);

    /**
      Performs tessalation like `tessalateSync` (@see tessalateSync), 
      however it performs the computation in a compute pipeline.
     @param params 
     @param viewport
     @returns std::future<TETessellationResult>
    */
    virtual std::future<TETessellationResult> tessalateOnGPU(const TETessellationParams & params, GEViewport * viewport = nullptr) = 0;

    /**
      Performs tessalation like `tessalateSync` (@see tessalateSync), 
      however it performs the computation in a seperate thread
     @param params 
     @param viewport
     @returns std::future<TETessellationResult>
    */
    std::future<TETessellationResult> tessalateAsync(const TETessellationParams & params, GEViewport * viewport = nullptr);

};

/**
 @brief The Omega Tessalation Engine
*/
class OMEGAGTE_EXPORT OmegaTessellationEngine {
public:
  /**
   NEVER CALL THIS FUNCTION! Please invoke GTE::Init()
  */
  static SharedHandle<OmegaTessellationEngine> Create();
    /**
        Create a Tessalation Context from a GENativeRenderTarget
        @param[in] renderTarget
        @returns SharedHandle<OmegaTessellationEngineContext>
    */
    SharedHandle<OmegaTessellationEngineContext> createTEContextFromNativeRenderTarget(SharedHandle<GENativeRenderTarget> & renderTarget);

    /**
        Create a Tessalation Context from a GETextureRenderTarget
        @param[in] renderTarget
        @returns SharedHandle<OmegaTessellationEngineContext>
    */
    SharedHandle<OmegaTessellationEngineContext> createTEContextFromTextureRenderTarget(SharedHandle<GETextureRenderTarget> & renderTarget);
};

_NAMESPACE_END_

#endif
