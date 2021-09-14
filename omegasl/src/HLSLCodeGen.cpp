#include "CodeGen.h"

#ifdef TARGET_DIRECTX
#include <d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")

#endif

namespace omegasl {

    class HLSLCodeGen : public CodeGen {
        std::ofstream shaderOut;
    public:
        HLSLCodeGen(CodeGenOpts &opts): CodeGen(opts){}
        void writeNativeStructDecl(ast::StructDecl *decl, std::ostream &out) override {

        }
        void generateExpr(ast::Expr *expr) override {

        }
        void generateBlock(ast::Block &block) override {

        }
        void generateDecl(ast::Decl *decl) override {
            switch (decl->type) {
                case STRUCT_DECL : {
                    writeNativeStructDecl((ast::StructDecl *)decl,shaderOut);
                    break;
                }
                case SHADER_DECL : {
                    auto _decl = (ast::ShaderDecl *)decl;
                    shaderOut.open(OmegaCommon::FS::Path(opts.outputDir).append(_decl->name).str() + ".hlsl");



                    shaderOut.close();
                    break;
                }
            }
        }
        void compileShader(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &name, const OmegaCommon::FS::Path &path, const OmegaCommon::FS::Path &outputPath) override {
            std::ostringstream out("dxc -nologo -T");
            if(type == ast::ShaderDecl::Vertex){
                out << "vs_5_0";
            }
            else if(type == ast::ShaderDecl::Fragment){
                out << "ps_5_0";
            }
            else if(type == ast::ShaderDecl::Compute){
                out << "cs_5_0";
            }
            out << "-Fo " << OmegaCommon::FS::Path(outputPath).append(name).absPath();
            out << " " << OmegaCommon::FS::Path(path).str();
        }

        void compileShaderOnRuntime(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &source, const OmegaCommon::StrRef &name) override {
#ifdef TARGET_DIRECTX

#endif
        }
    };

    std::shared_ptr<CodeGen> HLSLCodeGenMake(CodeGenOpts &opts){
        return std::make_shared<HLSLCodeGen>(opts);
    }




}