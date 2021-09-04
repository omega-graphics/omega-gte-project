#include <stdlib.h>

#ifndef omegasl_h
#define  omegasl_h

/// @name Standard *.omegasllib structs
/// @{
typedef const char * CString;

struct omegasl_lib_header {
    CString name;
    unsigned entryCount;
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
    omegasl_shader_layout_desc_io_mode io_mode;
    size_t location;
    size_t offset;
};

typedef enum : int {
    OMEGASL_SHADER_VERTEX,
    OMEGASL_SHADER_FRAGMENT,
    OMEGASL_SHADER_COMPUTE
} omegasl_shader_type;

struct omegasl_compute_shader_threadgroup_desc {
    unsigned x,y,z;
};

struct omegasl_shader {
    omegasl_shader_type type;
    omegasl_compute_shader_threadgroup_desc threadgroupDesc;
    omegasl_shader_layout_desc *pLayout;
    unsigned nLayout;
    void *data;
    size_t dataSize;
};

/// @}
     
#endif