#include "AST.h"
#include <memory>

#ifndef OMEGASL_CODEGEN_H
#define OMEGASL_CODEGEN_H

namespace omegasl {

    struct CodeGenOpts {
        bool emitHeaderOnly;
        OmegaCommon::StrRef outputDir;
    };

    class InterfaceGen;

    struct CodeGen {
        ast::SemFrontend *typeResolver;
        std::shared_ptr<InterfaceGen> interfaceGen;
        CodeGenOpts & opts;
        explicit CodeGen(CodeGenOpts & opts):opts(opts),typeResolver(nullptr),interfaceGen(std::make_shared<InterfaceGen>(opts.outputDir + "/structs.h",this)){ }
        void setTypeResolver(ast::SemFrontend *_typeResolver){ typeResolver = _typeResolver;}
        virtual void generateDecl(ast::Decl *decl) = 0;
        virtual void generateExpr(ast::Expr *expr) = 0;
        virtual void writeNativeStructDecl(ast::StructDecl *decl,std::ostream & out) = 0;
        virtual void compileShader(ast::ShaderDecl::Type type,const OmegaCommon::StrRef & name,const OmegaCommon::FS::Path & path,const OmegaCommon::FS::Path & outputPath) = 0;
    };

    class InterfaceGen final {
        std::ofstream out;
        CodeGen *gen;
    public:
        InterfaceGen(OmegaCommon::String filename,CodeGen *gen):gen(gen){
            out.open(filename);
            out << "// Warning! This file was generated by omegaslc. DO NOT EDIT!";
        }
        inline void writeCrossType(ast::TypeExpr *t){
            using namespace ast;
            auto *_t = gen->typeResolver->resolveTypeWithExpr(t);
            if(_t == builtins::void_type){
                out << "void";
            }
            else if(_t == builtins::float_type){
                out << "float";
            }
            if(t->pointer){
                out << " *";
            }
        }
        void generateStruct(ast::StructDecl *decl){
            out << "struct " << decl->name << " {" << std::endl;
            for(auto p : decl->fields){
                out << "    ";
                writeCrossType(p->typeExpr);
                out << " " << p->spec.name;
                out << ";" << std::endl;
            }
            out << "};" << std::endl;
#if defined(TARGET_DIRECTX)
            out << "#ifdef TARGET_DIRECTX";
#elif defined(TARGET_METAL)
            out << "#ifdef TARGET_METAL";
#endif
            gen->writeNativeStructDecl(decl,out);
            out << "#endif";

        }
        ~InterfaceGen(){
            out.close();
        }
    };

    struct MetalCodeOpts {

    };


    std::shared_ptr<CodeGen> GLSLCodeGenMake(CodeGenOpts &opts);
    std::shared_ptr<CodeGen> HLSLCodeGenMake(CodeGenOpts &opts);
    std::shared_ptr<CodeGen> MetalCodeGenMake(CodeGenOpts &opts);


}

#endif