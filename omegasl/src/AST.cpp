#include "AST.h"

#include "Toks.def"

namespace omegasl::ast {

    namespace builtins {
        Scope *global_scope = Scope::Create("global",nullptr);

        Type *void_type = new Type{KW_TY_VOID,global_scope};
        Type *int_type = new Type{KW_TY_INT,global_scope};

        Type *float_type = new Type{KW_TY_FLOAT,global_scope};
        Type *float2_type = new Type{KW_TY_FLOAT2,global_scope};
        Type *float3_type = new Type{KW_TY_FLOAT3,global_scope};
        Type *float4_type = new Type{KW_TY_FLOAT4,global_scope};

        Type *uint_type = new Type{KW_TY_UINT,global_scope};

        Type *buffer_type = new Type{KW_TY_BUFFER,global_scope,true,{"type"}};
        Type *texture1d_type = new Type{KW_TY_TEXTURE1D,global_scope};
        Type *texture2d_type = new Type{KW_TY_TEXTURE2D,global_scope};
        Type *texture3d_type = new Type{KW_TY_TEXTURE3D,global_scope};
    }

    Scope *Scope::Create(OmegaCommon::StrRef name, Scope *parent) {
        return new Scope{name,parent};
    }

    bool LiteralExpr::isFloat() const {
        return f_num.has_value();
    }

    bool LiteralExpr::isDouble() const {
        return d_num.has_value();
    }

    bool LiteralExpr::isInt() const {
        return i_num.has_value();
    }

    bool LiteralExpr::isUint() const {
        return ui_num.has_value();
    }

    bool LiteralExpr::isString() const {
        return str.has_value();
    }

    bool Scope::isParentScope(ast::Scope *scope) const {
        ast::Scope *current = parent;
        bool success = false;
        while(current != nullptr){
            current = parent->parent;
            if(scope == current){
                success = true;
                break;
            }
        }
        return success;
    }

    TypeExpr *TypeExpr::Create(OmegaCommon::StrRef name, bool pointer,bool hasTypeArgs,OmegaCommon::Vector<TypeExpr *> * args) {
        if(hasTypeArgs){
            return new TypeExpr {name, pointer,hasTypeArgs,*args};
        }
        else {
            return new TypeExpr {name,pointer,hasTypeArgs};
        }

    }

    TypeExpr *TypeExpr::Create(Type *type, bool pointer) {
        return new TypeExpr{type->name, pointer};
    }

    bool TypeExpr::compare(TypeExpr *other) {
       return (pointer == other->pointer) && (name == other->name);
    }

    TypeExpr::~TypeExpr() {
        if(hasTypeArgs)
            for(auto el : args){
                delete el;
            }
    }
}