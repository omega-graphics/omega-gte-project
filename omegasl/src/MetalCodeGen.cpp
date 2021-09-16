#include "CodeGen.h"



namespace omegasl {

    const char defaultHeaders[] = R"(// Warning! This file was generated by omegaslc
#include <metal_stdlib>
#include <simd/simd.h>

)";

    #ifdef TARGET_METAL
        void compileMTLShader(void *mtl_device,unsigned length,const char *string,void **pDest);
    #endif

    class MetalCodeGen : public CodeGen {
        void *mtl_device = nullptr;

        std::ofstream shaderOut;

        std::map<std::string,std::string> generatedFuncs;
        std::map<std::string,std::string> generatedStructs;

        MetalCodeOpts & metalCodeOpts;

    public:
        MetalCodeGen(CodeGenOpts &opts,MetalCodeOpts & metalCodeOpts): CodeGen(opts),metalCodeOpts(metalCodeOpts){

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
        inline void writeAttributeName(OmegaCommon::StrRef attributeName,std::ostream & out){
            if(attributeName == ATTRIBUTE_POSITION){
                out << "position";
            }
            else if(attributeName == ATTRIBUTE_VERTEX_ID){
                out << "vertex_id";
            }
        }
        void generateExpr(ast::Expr *expr) override {
            switch (expr->type) {
                case ID_EXPR : {
                    auto _expr = (ast::IdExpr *)expr;
                    shaderOut << _expr->id;
                    break;
                }
            }
        }
    private:
        unsigned level_count;
    public:
        void generateBlock(ast::Block &block) override {

            for(unsigned l = level_count;l != 0;l--){
                shaderOut << "    ";
            }
            shaderOut << "{" << std::endl;
            ++level_count;

            for(auto stmt : block.body){
                for(unsigned l = level_count;l != 0;l--){
                    shaderOut << "    ";
                }
                if(stmt->type & DECL){
                    generateDecl((ast::Decl *)stmt);
                    shaderOut << ";" << std::endl;
                }
                else {
                    generateExpr((ast::Expr *)stmt);
                    shaderOut << ";" << std::endl;
                }
            }

            --level_count;
            shaderOut << "}" << std::endl;
        }
        void generateDecl(ast::Decl *decl) override {
            switch (decl->type) {
                case VAR_DECL : {
                    auto *_decl = (ast::VarDecl *)decl;
                    writeTypeExpr(_decl->typeExpr,shaderOut);
                    shaderOut << " " << _decl->spec.name;
                    if(_decl->spec.initializer.has_value()){
                        shaderOut << " = ";
                        generateExpr(_decl->spec.initializer.value());
                    }
                    break;
                }
                case RETURN_DECL : {
                    auto *_decl = (ast::ReturnDecl *)decl;
                    shaderOut << "return ";
                    generateExpr(_decl->expr);
                    break;
                }
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
                            out << "[[";
                            writeAttributeName(p.attributeName.value(),out);
                            out << "]]";
                        }
                        out << ";" << std::endl;
                    }
                    out << "};" << std::endl;

                    generatedStructs.insert(std::make_pair(_decl->name,out.str()));
                    break;
                }
                case SHADER_DECL : {
                    level_count = 0;
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

                    unsigned i = 0,bufferCount = 0,textureCount = 0;
                    for(auto & res : _decl->resourceMap){
                        if(i != 0){
                            shaderOut << ",";
                        }
                        auto & res_desc = *(resourceStore.find(res.name));
                        auto type_ = typeResolver->resolveTypeWithExpr(res_desc->typeExpr);

                        if(res.access == ast::ShaderDecl::ResourceMapDesc::In){
                            shaderOut << "constant ";
                        }
                        else {
                            shaderOut << "device ";
                        }

                        bool isTexture = false,isBuffer = false;

                        if(type_ == ast::builtins::buffer_type){
                            isBuffer = true;
                            writeTypeExpr(res_desc->typeExpr->args[0],shaderOut);
                            shaderOut << " *";
                        }
                        else if(type_ == ast::builtins::texture1d_type){
                            isTexture = true;
                            shaderOut << "texture1d<half,";
                        }

                        if(isTexture){
                            if(res.access == ast::ShaderDecl::ResourceMapDesc::In){
                                shaderOut << "access::read>";
                            }
                            else if(res.access == ast::ShaderDecl::ResourceMapDesc::Inout){
                                shaderOut << "access::readwrite>";
                            }
                            else {
                                shaderOut << "access::write>";
                            }
                        }

                        shaderOut << " " << res_desc->name;

                        if(isTexture){
                            shaderOut << "[[texture(" << textureCount;
                            ++textureCount;
                            shaderOut << ")]]";
                        }
                        else if(isBuffer){
                            shaderOut << "[[buffer(" << bufferCount;
                            ++bufferCount;
                            shaderOut << ")]]";
                        }
                        i++;
                    }

                    if(!(_decl->params.empty() && _decl->resourceMap.empty())){
                        shaderOut << ",";
                    }

                    for(auto p_it =  _decl->params.begin();p_it != _decl->params.end();p_it++){

                        if(p_it != _decl->params.begin()) {
                            shaderOut << ",";
                        }

                        auto & p = *p_it;

                        writeTypeExpr(p.typeExpr,shaderOut);
                        shaderOut << " " << p.name << " ";
                        if(p.attributeName.has_value()){
                            shaderOut << "[[";
                            writeAttributeName(p.attributeName.value(),shaderOut);
                            shaderOut << "]]";
                        }
                    }
                    shaderOut << ")";

                    generateBlock(*_decl->block);

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
            std::ostringstream out;

            out << metalCodeOpts.metal_cmd;
            auto object_file = OmegaCommon::FS::Path(outputPath).append(name).concat(".air").str();
            out << " -o " << object_file << " -c " << OmegaCommon::FS::Path(path).append(name).concat(".metal").str();

            std::cout << "Exec:" << out.str() << std::endl;

            std::system(out.str().c_str());
            out.str("");
            out << metalCodeOpts.metallib_cmd;
            out << " -o " << OmegaCommon::FS::Path(outputPath).append(name).concat(".metallib").str() << " " << object_file;

            std::cout << "Exec:" << out.str() << std::endl;
            std::system(out.str().c_str());
        }
        void compileShaderOnRuntime(ast::ShaderDecl::Type type,const OmegaCommon::StrRef & source,const OmegaCommon::StrRef &name) override {
            #ifdef TARGET_METAL
                if(mtl_device != nullptr){
                    auto & _m = shaderMap[name.data()];
                    compileMTLShader(mtl_device,source.size(),source.data(), &_m.data);
                }
            #endif
        }
    };

    std::shared_ptr<CodeGen> MetalCodeGenMake(CodeGenOpts &opts,MetalCodeOpts &metalCodeOpts){
        return std::make_shared<MetalCodeGen>(opts,metalCodeOpts);
    };
}
