#include "GTEBase.h"

#ifndef OMEGAGTE_GTESHADER_H
#define OMEGAGTE_GTESHADER_H

_NAMESPACE_BEGIN_

typedef struct __GTEFunctionInternal GTEShader;

struct GTEShaderLibrary {
    std::map<std::string,SharedHandle<GTEShader>> shaders;
};

namespace Shader {
    typedef enum : int {
        float_1,
        float_2,
        float_3,
        float_4,
        float_1x2,
        float_2x2,
        float_3x2,
        float_4x2,
        float_1x3,
        float_2x3,
        float_3x3,
        float_4x3,
        float_1x4,
        float_2x4,
        float_3x4,
        float_4x4,
    } DataType;

    typedef enum : int {
        Add,
        Subtract,
        Multiply,
        Divide,
        Dot,
        Cross
    } OpType;

    typedef enum : int {
        Vertex,
        Fragment,
        Compute,
    } Type;
}

// class GTEShaderBuilder {
//     void startShader(Shader::Type type);
//     void create(TStrRef name,Shader::DataType type);
//     void set(TStrRef name);
//     void op(TStrRef var1,TStrRef var2,TStrRef var_result,Shader::OpType op);
//     void invoke(TStrRef func,ArrayRef<TStrRef> args);
//     SharedHandle<GTEShader> finishShader();
// };

_NAMESPACE_END_

#endif