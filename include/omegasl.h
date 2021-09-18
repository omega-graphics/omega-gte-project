#include <stdlib.h>

#ifndef omegasl_h
#define  omegasl_h

/// @name Standard *.omegasllib structs
/// @{
typedef const char * CString;

struct omegasl_lib_header {
    unsigned name_length;
    CString name;
    unsigned entry_count;
};

typedef enum : int {
    OMEGASL_SHADER_DESC_IO_IN,
    OMEGASL_SHADER_DESC_IO_OUT,
    OMEGASL_SHADER_DESC_IO_INOUT
} omegasl_shader_layout_desc_io_mode;

typedef enum : int {
    OMEGASL_SHADER_CONSTANT_DESC,
    OMEGASL_SHADER_BUFFER_DESC,
    OMEGASL_SHADER_TEXTURE1D_DESC,
    OMEGASL_SHADER_TEXTURE2D_DESC,
    OMEGASL_SHADER_TEXTURE3D_DESC,
    OMEGASL_SHADER_SAMPLER2D_DESC,
    OMEGASL_SHADER_SAMPLER3D_DESC
} omegasl_shader_layout_desc_type;

struct omegasl_shader_layout_desc {
    omegasl_shader_layout_desc_type type;
    unsigned gpu_relative_loc;
    omegasl_shader_layout_desc_io_mode io_mode;
    size_t location;
    size_t offset;
};

typedef enum : int {
    OMEGASL_SHADER_VERTEX,
    OMEGASL_SHADER_FRAGMENT,
    OMEGASL_SHADER_COMPUTE
} omegasl_shader_type;

typedef enum : int {
    //Int Types
    OMEGASL_INT,
    OMEGASL_INT2,
    OMEGASL_INT3,
    OMEGASL_INT4,

    //Uint Types
    OMEGASL_UINT,
    OMEGASL_UINT2,
    OMEGASL_UINT3,
    OMEGASL_UINT4,

    //Float Types
    OMEGASL_FLOAT,
    OMEGASL_FLOAT2,
    OMEGASL_FLOAT3,
    OMEGASL_FLOAT4,
    OMEGASL_FLOAT2x1,
    OMEGASL_FLOAT2x2,
    OMEGASL_FLOAT2x3,
    OMEGASL_FLOAT2x4,
    OMEGASL_FLOAT3x1,
    OMEGASL_FLOAT3x2,
    OMEGASL_FLOAT3x3,
    OMEGASL_FLOAT3x4,
    OMEGASL_FLOAT4x1,
    OMEGASL_FLOAT4x2,
    OMEGASL_FLOAT4x3,
    OMEGASL_FLOAT4x4

} omegasl_data_type;

struct omegasl_vertex_shader_param_desc {
    CString name;
    omegasl_data_type type;
    size_t offset;
};

struct omegasl_vertex_shader_input_desc {
    bool useVertexID;
    omegasl_vertex_shader_param_desc *pParams;
    unsigned nParam;
};

struct omegasl_compute_shader_threadgroup_desc {
    unsigned x,y,z;
};


struct omegasl_shader {
    omegasl_shader_type type;
    CString name;
    omegasl_vertex_shader_input_desc vertexShaderInputDesc;
    omegasl_compute_shader_threadgroup_desc threadgroupDesc;
    omegasl_shader_layout_desc *pLayout;
    unsigned nLayout;
    void *data;
    size_t dataSize;
};

/// @}


/// Build Library
#ifdef RUNTIME_SHADER_COMP_SUPPORT

#include <omega-common/common.h>


struct omegasl_shader_lib {
    omegasl_lib_header header;
    omegasl_shader *shaders;
};

/**
 *  @brief OmegaSL Compiler Frontend. (For Runtime Compilation Use Only)
 *  @paragraph
 *  The biggest difference between this and the `omegaslc` executable
 *  is that it invokes the target platform's runtime shader compiler
 *  (D3DCompileFromFile on Windows,
    \c \# id<MTLLibrary> [(id<MTLDevice>) newlibraryFromSource: withCompletionProvider:]
    on Apple Devices,
 *  and shaderc::CompilerGlslToSpv() on Android and Linux.)
 * */
class OmegaSLCompiler {
public:
    struct Source {
        static std::shared_ptr<Source> fromFile(OmegaCommon::FS::Path path);
        static std::shared_ptr<Source> fromString(OmegaCommon::String & buffer);
    };
    static std::shared_ptr<OmegaSLCompiler> Create();
    virtual std::shared_ptr<omegasl_shader_lib> compile(std::initializer_list<std::shared_ptr<Source>> sources) = 0;
};

#endif

#endif