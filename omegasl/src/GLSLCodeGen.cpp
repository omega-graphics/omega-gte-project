#include "CodeGen.h"




namespace omegasl {
    class GLSLCodeGen : public CodeGen {
    public:
        GLSLCodeGen(CodeGenOpts &opts): CodeGen(opts){

        }
        void writeNativeStructDecl(ast::StructDecl *decl, std::ostream &out) override {

        }
        void generateDecl(ast::Decl *decl) override{

        }
        void generateExpr(ast::Expr *expr) override{

        }
        void compileShader(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &name, const OmegaCommon::FS::Path &path, const OmegaCommon::FS::Path &outputPath) override {

        }
    };

    std::shared_ptr<CodeGen> GLSLCodeGenMake(CodeGenOpts &opts){
        return std::make_shared<GLSLCodeGen>(opts);
    }
}