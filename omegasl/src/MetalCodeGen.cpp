#include "CodeGen.h"

namespace omegasl {

    const char defaultHeaders[] = R"(// Warning! This file was generated by omegaslc
#include <metal_stdlib>
#include <simd/simd.h>

)";

    class MetalCodeGen : public CodeGen {
        std::ofstream shaderOut;

        std::map<std::string,std::string> generatedFuncs;
        std::map<std::string,std::string> generatedStructs;

    public:
        MetalCodeGen(CodeGenOpts &opts): CodeGen(opts){

        }
        inline void writeTypeExpr(ast::TypeExpr *t,std::ostream & out){
            using namespace ast;
            auto * _t = typeResolver->resolveTypeWithExpr(t);

            if(_t == builtins::void_type){
                out << "void";
            }
            else if(_t == builtins::int_type){
                out << "int";
            }
            else if(_t == builtins::uint_type){
                out << "uint";
            }
            else if(_t == builtins::float_type){
                out << "simd_float";
            }
            else if(_t == builtins::float2_type){
                out << "simd_float2";
            }
            else if(_t == builtins::float3_type){
                out << "simd_float3";
            }
            else if(_t == builtins::float4_type){
                out << "simd_float4";
            }
            else {
                out << t->name;
            }


            if(t->pointer){
                out << " *";
            }
        }
        void writeExpr(ast::Expr *expr,std::ostream & out){

        }
        void generateExpr(ast::Expr *expr) override {
            writeExpr(expr,shaderOut);
        }
        void generateDecl(ast::Decl *decl) override {
            switch (decl->type) {
                case RESOURCE_DECL : {
                    auto *_decl = (ast::ResourceDecl *)decl;
                    resourceStore.add(_decl);
                    break;
                }
                case STRUCT_DECL : {
                    auto *_decl = (ast::StructDecl *)decl;
                    std::ostringstream out;
                    out << "struct " << _decl->name << " {" << std::endl;
                    for(auto p : _decl->fields){
                        out << "    ";
                        writeTypeExpr(p.typeExpr,out);
                        out << " " << p.name;
                        if(p.attributeName.has_value()){
                            out << "[[" << p.attributeName.value() << "]]";
                        }
                        out << ";" << std::endl;
                    }
                    out << "};" << std::endl;

                    generatedStructs.insert(std::make_pair(_decl->name,out.str()));
                    break;
                }
                case SHADER_DECL : {
                    auto *_decl = (ast::ShaderDecl *)decl;
                    shaderOut.open(OmegaCommon::String(opts.tempDir) + "/" + _decl->name + ".metal",std::ios::out);
                    shaderOut << defaultHeaders;

                    std::vector<std::string> used_type_list;
                    typeResolver->getStructsInFuncDecl(_decl,used_type_list);

                    if(!used_type_list.empty()){
                        for(auto & t : used_type_list){
                            shaderOut << generatedStructs[t] << std::endl << std::endl;
                        }
                    }

                    if(_decl->shaderType == ast::ShaderDecl::Vertex){
                        shaderOut << "vertex";
                    }
                    else if(_decl->shaderType == ast::ShaderDecl::Fragment){
                        shaderOut << "fragment";
                    }
                    else if(_decl->shaderType == ast::ShaderDecl::Compute){
                        shaderOut << "kernel";
                    }

                    shaderOut << " ";
                    writeTypeExpr(_decl->returnType,shaderOut);
                    shaderOut << " " << _decl->name << " ";
                    shaderOut << "(";
                    for(auto p_it =  _decl->params.begin();p_it != _decl->params.end();p_it++){

                        if(p_it != _decl->params.begin()) {
                            shaderOut << ",";
                        }

                        auto & p = *p_it;

                        writeTypeExpr(p.typeExpr,shaderOut);
                        shaderOut << " " << p.name << " ";
                        if(p.attributeName.has_value()){
                            shaderOut << "[[";
                            if(p.attributeName == ATTRIBUTE_VERTEX_ID){
                                shaderOut << "vertex_id";
                            }
                            shaderOut << "]]";
                        }
                    }
                    shaderOut << ")";


                    shaderOut.close();
                    break;
                }
                default : {
                    break;
                }
            }
        }
        void writeNativeStructDecl(ast::StructDecl *decl, std::ostream &out) override {
            out << "struct " << decl->name << "_ {" << std::endl;
            for(auto p : decl->fields){
                out << "    " << std::flush;
                writeTypeExpr(p.typeExpr,out);
                out << " " << p.name << " " << std::flush;
//
                out << ";" << std::endl;
            }
            out << "};" << std::endl;
        }
        void compileShader(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &name, const OmegaCommon::FS::Path &path,const OmegaCommon::FS::Path & outputPath) override {
            std::ostringstream out("xcrun metal ");
            out << "-o " << OmegaCommon::FS::Path(outputPath).append(name).str() << ".air " << "-c " << OmegaCommon::FS::Path(path).str();
            std::system(out.str().c_str());
            out.str("xcrun metallib");
            out << "-o " << OmegaCommon::FS::Path(outputPath).append(name).str() << ".metallib " << OmegaCommon::FS::Path(path).str();
            std::system(out.str().c_str());
        }
    };

    std::shared_ptr<CodeGen> MetalCodeGenMake(CodeGenOpts &opts){
        return std::make_shared<MetalCodeGen>(opts);
    };
}