#include "AST.def"

#include <omega-common/common.h>

#ifndef OMEGASL_AST_H
#define OMEGASL_AST_H

namespace omegasl {

    namespace ast {
        /// @brief Defines a Scope in the AST.
        struct Scope {
            OmegaCommon::String name;
            Scope *parent;
            /** @brief Recursively checks to see if this scope is a child of the provided Scope ```scope```.
             * @param scope the scope
             * @returns bool
             * */
            bool isParentScope(Scope *scope) const;
            static Scope *Create(OmegaCommon::StrRef name, Scope *parent);
        };


        struct Type {
            OmegaCommon::String name;
            Scope *declaredScope;
            bool builtin = true;
//           std::vector<std::string> typeArgs;
        };

        namespace builtins {
            extern Scope *global_scope;
#define DECLARE_BUILTIN_TYPE(name) extern Type *name

            DECLARE_BUILTIN_TYPE(void_type);
            DECLARE_BUILTIN_TYPE(int_type);
            DECLARE_BUILTIN_TYPE(uint_type);
            DECLARE_BUILTIN_TYPE(float_type);
            DECLARE_BUILTIN_TYPE(float2_type);
            DECLARE_BUILTIN_TYPE(float3_type);
            DECLARE_BUILTIN_TYPE(float4_type);

            DECLARE_BUILTIN_TYPE(buffer_type);
            DECLARE_BUILTIN_TYPE(texture1d_type);
            DECLARE_BUILTIN_TYPE(texture2d_type);
            DECLARE_BUILTIN_TYPE(texture3d_type);
        }

        /// @brief Refers to a type that already exists.
        struct TypeExpr {
            OmegaCommon::String name;
            bool pointer;

            static TypeExpr *Create(OmegaCommon::StrRef name, bool pointer = false);
            bool compare(TypeExpr *other);
        };

        struct FuncDecl;
        struct StructDecl;

        /// @brief Provides useful semantics info about AST Nodes.
        /// @paragraph This includes resolving ast::Type using ast::TypeExpr
        /// and retrieving StructDecls used in a ShaderDecl
        class SemFrontend {
        public:
            virtual void getStructsInFuncDecl(FuncDecl *funcDecl,std::vector<std::string> & out) = 0;
            /** @brief Retrieves the underlying Type associated with this TypeExpr
             * @param expr The TypeExpr to evalutate.
             * @returns Type **/
            virtual Type *resolveTypeWithExpr(TypeExpr *expr) = 0;
        };

        /// @brief A Node in the AST
        struct Stmt {
            ASTType type;
            Scope *scope;
        };

        struct Expr;

        struct Decl : public Stmt {};

        struct ResourceDecl : public Decl {
            TypeExpr *typeExpr;
            OmegaCommon::String name;
            size_t registerNumber;
        };

        struct AttributedFieldDecl {
            TypeExpr *typeExpr;
            OmegaCommon::String name;
            std::optional<OmegaCommon::String> attributeName;
        };

        struct VarDecl : public Decl {
            TypeExpr *typeExpr;
            struct Spec {
                OmegaCommon::String name;
                std::optional<ast::Expr *> initializer;
            } spec;
        };

        struct Block {
            std::vector<ast::Stmt *> body;
        };

        /// @brief Declares a Struct.
        /// @paragraph Can be either public or for shader internal use only
        /// by marking it with the `internal` keyword)
        struct StructDecl : public Decl {
            OmegaCommon::String name;
            bool internal;
            std::vector<AttributedFieldDecl> fields;
        };

        /// @brief Declares a Function
        struct FuncDecl : public Decl {
            OmegaCommon::String name;
            OmegaCommon::Vector<AttributedFieldDecl> params;
            TypeExpr *returnType;
            std::unique_ptr<ast::Block> block;
        };

        /// @brief Declares a Shader.
        struct ShaderDecl : public FuncDecl {
            typedef enum : int {
                Vertex,
                Fragment,
                Compute
            } Type;
            Type shaderType;
            struct ResourceMapDesc {
                typedef enum : int {
                    In,
                    Out,
                    Inout
                } Access;
                Access access;
                OmegaCommon::String name;
            };
            OmegaCommon::Vector<ResourceMapDesc> resourceMap;
        };

        struct Expr : public Stmt {};

        struct IdExpr : public Expr {
            OmegaCommon::String id;
        };

        struct LiteralExpr : public Expr {

            /// Num Literal
            std::optional<float> f_num;
            std::optional<int> i_num;
            std::optional<unsigned int> ui_num;
            std::optional<double> d_num;
            /// Str Literal
            std::optional<OmegaCommon::String> str;

            bool isFloat() const;
            bool isInt() const;
            bool isUint() const;
            bool isDouble() const;

            bool isString() const;
        };

        struct MemberExpr : public Expr {
            Expr *lhs;
            OmegaCommon::String rhs_id;
        };

        struct ArrayExpr : public Expr {
            std::vector<Expr *> elm;
        };

        struct CallExpr : public Expr {
            Expr *callee;
            std::vector<Expr *> args;
        };

        struct UnaryOpExpr : public Expr {
            bool isPrefix;
            OmegaCommon::String op;
            Expr *expr;
        };

        struct BinaryExpr : public Expr {
            OmegaCommon::String op;
            Expr *lhs;
            Expr *rhs;
        };

        struct PointerExpr : public Expr {
            typedef enum : int {
                AddressOf,
                Dereference
            } Type;
            Type ptType;
            Expr *expr;
        };

    }
}
#endif