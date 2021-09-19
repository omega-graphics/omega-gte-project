#include "CodeGen.h"

#ifdef TARGET_DIRECTX
#include <d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")

#endif

namespace omegasl {

    class HLSLCodeGen final : public CodeGen {
        std::ofstream shaderOut;
        HLSLCodeOpts & hlslCodeOpts;

        OmegaCommon::Map<OmegaCommon::String,OmegaCommon::String> generatedStructs;

    public:
        HLSLCodeGen(CodeGenOpts &opts,HLSLCodeOpts & hlslCodeOpts): CodeGen(opts),hlslCodeOpts(hlslCodeOpts){}
        void generateExpr(ast::Expr *expr) override {

        }
        void generateBlock(ast::Block &block) override {

        }
        inline void writeAttribute(OmegaCommon::StrRef attributeName,std::ostream & out){
            if(attributeName == ATTRIBUTE_VERTEX_ID){
                out << "SV_VertexID";
            }
            else if(attributeName == ATTRIBUTE_COLOR){
                out << "COLOR";
            }
        }
        inline void writeTypeExpr(ast::TypeExpr *typeExpr,std::ostream & out){
            auto _ty = typeResolver->resolveTypeWithExpr(typeExpr);
            if(_ty == ast::builtins::float_type){
                out << "float";
            }
            else if(_ty == ast::builtins::float2_type){
                out << "float2";
            }
            else if(_ty == ast::builtins::float3_type){
                out << "float3";
            }
            else if(_ty == ast::builtins::float4_type){
                out << "float4";
            }
            else {
                out << _ty->name;
            }


            if(typeExpr->pointer){
                out << "*";
            }


        }
        void generateDecl(ast::Decl *decl) override {
            switch (decl->type) {
                case STRUCT_DECL : {
                    auto _decl = (ast::StructDecl *)decl;
                    std::ostringstream out;
                    out << "struct " << _decl->name << "{" << std::endl;
                    for(auto & f : _decl->fields){
                        out << "    " << std::flush;
                        writeTypeExpr(f.typeExpr,out);
                        out << " " << f.name;
                        if(f.attributeName.has_value()){
                            out << ":";
                            writeAttribute(f.attributeName.value(),out);
                        }
                        out << ";" << std::endl;
                    }
                    out << "};" << std::endl;

                    generatedStructs.insert(std::make_pair(_decl->name,out.str()));

                    break;
                }
                case SHADER_DECL : {
                    auto _decl = (ast::ShaderDecl *)decl;
                    shaderOut.open(OmegaCommon::FS::Path(opts.outputLib).append(_decl->name).concat(".hlsl").absPath());



                    shaderOut.close();
                    break;
                }
            }
        }
        void compileShader(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &name, const OmegaCommon::FS::Path &path, const OmegaCommon::FS::Path &outputPath) override {
            std::ostringstream out(" -nologo -T");
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

            auto dxc_process = OmegaCommon::ChildProcess::OpenWithStdoutPipe(hlslCodeOpts.dxc_cmd,out.str().c_str());
            auto res = dxc_process.wait();
        }

        void compileShaderOnRuntime(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &source, const OmegaCommon::StrRef &name) override {
#ifdef TARGET_DIRECTX
            ID3DBlob *blob;
            OmegaCommon::String target;
            if(type == ast::ShaderDecl::Vertex){
                target = "vs_5_0";
            }
            else if(type == ast::ShaderDecl::Fragment){
                target = "ps_5_0";
            }
            else if(type == ast::ShaderDecl::Compute){
                target = "cs_5_0";
            }

            D3DCompile(source.data(),
                       source.size(),
                       name.data(),
                       nullptr,
                       D3D_COMPILE_STANDARD_FILE_INCLUDE,
                       name.data(),
                       target.data(),
                       NULL,
                       NULL
                       ,&blob,
                       nullptr);

            auto & shaderEntry = shaderMap[name.data()];
            shaderEntry.data = blob->GetBufferPointer();
            shaderEntry.dataSize = blob->GetBufferSize();
#endif
        }
    };

    std::shared_ptr<CodeGen> HLSLCodeGenMake(CodeGenOpts &opts,HLSLCodeOpts & hlslCodeOpts){
        return std::make_shared<HLSLCodeGen>(opts,hlslCodeOpts);
    }




}