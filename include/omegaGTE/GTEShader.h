#include "GTEBase.h"

#ifndef OMEGAGTE_GTESHADER_H
#define OMEGAGTE_GTESHADER_H

_NAMESPACE_BEGIN_

struct GTEShader;

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

class OmegaGraphicsEngine;

class OMEGAGTE_EXPORT GTEShaderBuilder {
    OmegaGraphicsEngine *engine;
    public:

    GTEShaderBuilder(OmegaGraphicsEngine *engine);

    std::ostringstream out;

    public:
    GTEShaderBuilder & startShader(Shader::Type type);
    GTEShaderBuilder & createVariable(const StrRef & name, Shader::DataType type);
    
    class OMEGAGTE_EXPORT Expression {
        std::ostringstream out;
    public:
        Expression();
        Expression & id(StrRef id);
        Expression & float_literal(float f);
        Expression & int_literal(int i);
        Expression & string_literal(const StrRef & str);
        Expression & op(Shader::OpType type,Expression &lhs,Expression &rhs);
        Expression & array(std::vector<Expression> exprs);
    };
    
    GTEShaderBuilder & exprStmt(const Expression &expr);
    GTEShaderBuilder & setVariable(const StrRef & name, const Expression & expr);
   
    SharedHandle<GTEShader> finishShader();
};

// void end(){
//     GTEShaderBuilder builder;
//     auto shader = builder.startShader(Shader::Vertex).
//     createVariable("myVariable",Shader::float_1).
//     setVariable("myVariable",GTEShaderBuilder::Expression().float_literal(1.f)).
//     finishShader();
// }

_NAMESPACE_END_

#endif