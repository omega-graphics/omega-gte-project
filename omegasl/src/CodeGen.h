#include "AST.h"
#include <omegasl.h>
#include <memory>

#ifndef OMEGASL_CODEGEN_H
#define OMEGASL_CODEGEN_H

namespace omegasl {

    struct CodeGenOpts {
        bool emitHeaderOnly;
        bool runtimeCompile;
        OmegaCommon::StrRef outputDir;
        OmegaCommon::StrRef tempDir;
    };

    class InterfaceGen;

    struct CodeGen {
        struct ResourceStore {
        private:
            typedef std::vector<ast::ResourceDecl *> data ;
            data resources;
        public:
            inline void add(ast::ResourceDecl * res){
                resources.push_back(res);
            }
            inline data::iterator begin(){
                return resources.begin();
            }
            inline data::iterator find(const OmegaCommon::StrRef & name){
                using namespace OmegaCommon;
                auto it = resources.begin();
                for(;it != resources.end();it++){
                    auto & item = *it;
                    if(item->name == name){
                        break;
                    }
                }
                return it;
            };
            inline data::iterator end(){
                return resources.end();
            }
        };

        ResourceStore resourceStore;

        ast::SemFrontend *typeResolver;
        std::shared_ptr<InterfaceGen> interfaceGen;

        OmegaCommon::Map<OmegaCommon::String,omegasl_shader> shaderMap;

        CodeGenOpts & opts;
        explicit CodeGen(CodeGenOpts & opts):opts(opts),typeResolver(nullptr),interfaceGen(std::make_shared<InterfaceGen>(opts.outputDir + "/structs.h",this)){ }
        void setTypeResolver(ast::SemFrontend *_typeResolver){ typeResolver = _typeResolver;}
        virtual void generateDecl(ast::Decl *decl) = 0;
        virtual void generateExpr(ast::Expr *expr) = 0;
        virtual void generateBlock(ast::Block &block) = 0;
        virtual void writeNativeStructDecl(ast::StructDecl *decl,std::ostream & out) = 0;
        /** @brief Compiles the shader with the provided name and outputs the compiled version to the output path provided.
         * @param type The Shader Type
         * @param name The Shader Name
         * @param path The source file location.
         * @param outputPath The output file location.
         * */
        virtual void compileShader(ast::ShaderDecl::Type type,const OmegaCommon::StrRef & name,const OmegaCommon::FS::Path & path,const OmegaCommon::FS::Path & outputPath) = 0;
        /** @brief Compiles the Shader with the provided name and outputs the compiled version to the shadermap.
         * @param type The Shader Type
         * @param name The Shader Name
         * @note
         * This function is only called when compiling omegasl on runtime.
         * */
        virtual void compileShaderOnRuntime(ast::ShaderDecl::Type type,const OmegaCommon::StrRef & source,const OmegaCommon::StrRef & name) = 0;
        void linkShaderObjects(const OmegaCommon::StrRef & libname){
            std::ofstream out(OmegaCommon::FS::Path(opts.outputDir).append(libname).absPath() + ".omegasllib",std::ios::out | std::ios::binary);
            out.write((char *)&libname.size(),sizeof(OmegaCommon::StrRef::size_type));
            out.write(libname.data(),libname.size());
            unsigned int s = shaderMap.size();
            out.write((char *)&s,sizeof(s));

            for(auto & p : shaderMap){
                auto & shader_data = p.second;
                //1.  Write Shader Type
                out.write((char *)&shader_data.type,sizeof(shader_data.type));

                //2.  Write Shader Name Size and Name
                size_t shader_name_size = strlen(shader_data.name);
                out.write((char *)&shader_name_size,sizeof(shader_name_size));
                out.write( shader_data.name,std::make_signed_t<decltype(shader_name_size)>(shader_name_size));
                //3.  Write Shader Data Size and Data
                std::ifstream in(p.first,std::ios::in | std::ios::binary);
                in.seekg(0,std::ios::end);
                size_t dataSize = in.tellg();
                in.seekg(0,std::ios::beg);
                out.write((char *)&dataSize,sizeof(dataSize));
                while(!in.eof()){
                    out << in.get();
                }
                //4. Write Shader Layout Length and Data
                OmegaCommon::ArrayRef<omegasl_shader_layout_desc> layoutDescArr {shader_data.pLayout,shader_data.pLayout + shader_data.nLayout};
                out.write((char *)&shader_data.nLayout,sizeof(shader_data.nLayout));
                for(auto & layout : layoutDescArr){
                    out.write((char *)&layout,sizeof(layout));
                }

            }
            out.close();
        };
#ifdef RUNTIME_SHADER_COMP_SUPPORT
        std::shared_ptr<omegasl_shader_lib> getLibrary(OmegaCommon::StrRef name){
            auto res = std::make_shared<omegasl_shader_lib>();
            res->header.name = name.data();
            res->header.name_length = name.size();
            res->header.entry_count = shaderMap.size();
            res->shaders = new omegasl_shader [shaderMap.size()];
            unsigned idx = 0;
            for(auto & s_pair : shaderMap){
                memcpy(res->shaders + idx,&s_pair.second,sizeof(omegasl_shader));
            }
        }
        void resetShaderMap(){
            shaderMap.clear();
        }
#endif
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
                writeCrossType(p.typeExpr);
                out << " " << p.name;
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
        OmegaCommon::String metal_cmd;
        OmegaCommon::String metallib_cmd;
        void *mtl_device = nullptr;
    };


    std::shared_ptr<CodeGen> GLSLCodeGenMake(CodeGenOpts &opts);
    std::shared_ptr<CodeGen> HLSLCodeGenMake(CodeGenOpts &opts);
    std::shared_ptr<CodeGen> MetalCodeGenMake(CodeGenOpts &opts,MetalCodeOpts &metalCodeOpts);


}

#endif