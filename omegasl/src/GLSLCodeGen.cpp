#include "AST.def"
#include "CodeGen.h"
#include <sstream>

#ifdef TARGET_VULKAN
#include <shaderc/shaderc.h>
#endif

#define GLSL_VERTEX_ID "gl_VertexIndex"
#define GLSL_POSITION "gl_Position"


namespace omegasl {

    const char vertex_shader_ext[] = ".vert";
    const char fragment_shader_ext[] = ".frag";
    const char compute_shader_ext[] = ".comp";

    class GLSLCodeGen : public CodeGen {

        std::ofstream shaderOut;
        #ifdef TARGET_VULKAN
        shaderc_compiler_t compiler;
        #endif

        OmegaCommon::Map<OmegaCommon::String,OmegaCommon::String> generatedStructs;

        GLSLCodeOpts &glslCodeOpts;



    public:
        explicit GLSLCodeGen(CodeGenOpts &opts,GLSLCodeOpts &glslCodeOpts): CodeGen(opts),glslCodeOpts(glslCodeOpts){
            #ifdef TARGET_VULKAN
            compiler = shaderc_compiler_initialize();
            #endif
        }
        void writeNativeStructDecl(ast::StructDecl *decl, std::ostream &out) override {

        }
        inline void writeTypeExpr(ast::TypeExpr *typeExpr,std::ostream & out) {
            auto t = typeResolver->resolveTypeWithExpr(typeExpr);
            if(t == ast::builtins::void_type){
                out << "void";
            }
            else if(t == ast::builtins::float_type){
                out << "float";
            }
            else if(t == ast::builtins::float2_type){
                out << "vec2";
            }
            else if(t == ast::builtins::float3_type){
                out << "vec3";
            }
            else if(t == ast::builtins::float4_type){
                out << "vec4";
            }
            else if(t == ast::builtins::int_type){
                out << "int";
            }
            else if(t == ast::builtins::uint_type){
                out << "uint";
            }
            else {
                out << t->name;
            }

            if(typeExpr->pointer){
                out << " * ";
            }
        }
    private:
        unsigned indentLevel = 0;
    public:

        void generateBlock(ast::Block &block) override {
            shaderOut << "{" << std::endl;
            indentLevel += 1;
            for(auto stmt_it = block.body.begin();stmt_it != block.body.end();stmt_it++){
                auto stmt = *stmt_it;
                for(unsigned i = 0;i < indentLevel;i++){
                    shaderOut << "  ";
                }
                if(stmt->type & DECL){
                    generateDecl((ast::Decl *)stmt);
                }
                else {
                    generateExpr((ast::Expr *)stmt);
                }
                shaderOut << ";" << std::endl; 
            }
            indentLevel -= 1;
            shaderOut << "}" << std::endl;
        }
        void generateDecl(ast::Decl *decl) override{
            switch (decl->type) {
                case STRUCT_DECL : {
                    auto _decl = (ast::StructDecl *)decl;
                    std::ostringstream out;
                    out << "struct " << _decl->name << " {" << std::endl;
                    for(auto & f : _decl->fields){
                        out << "  ";
                        writeTypeExpr(f.typeExpr,out);
                        out << " " << f.name << ";" << std::endl;
                    }
                    out << "};";
                    generatedStructs.insert(std::make_pair(_decl->name,out.str()));
                    break;
                }
                case VAR_DECL : {
                    auto _decl = (ast::VarDecl *)decl;
                    writeTypeExpr(_decl->typeExpr,shaderOut);
                    shaderOut << " " << _decl->spec.name;
                    if(_decl->spec.initializer.has_value()){
                        shaderOut << " = " << std::flush;
                        generateExpr(_decl->spec.initializer.value());
                    }
                    break;
                }
                case SHADER_DECL : {

                    #define FRAGMENT_SHADER_OUTPUT_COLOR_NAME "__outColor";

                    auto _decl = (ast::ShaderDecl *)decl;

                    const char *file_ext = nullptr;
                    OmegaCommon::String return_val_replacement;

                    switch (_decl->shaderType) {
                        case ast::ShaderDecl::Vertex : {
                            file_ext = vertex_shader_ext;
                            return_val_replacement = GLSL_POSITION;
                            break;
                        }
                        case ast::ShaderDecl::Fragment : {
                            file_ext = fragment_shader_ext;
                            return_val_replacement = FRAGMENT_SHADER_OUTPUT_COLOR_NAME;
                            break;
                        }
                        case ast::ShaderDecl::Compute : {
                            file_ext = compute_shader_ext;
                            break;
                        }

                    }

                    std::cout << OmegaCommon::FS::Path(opts.tempDir).absPath() << std::endl;

                    auto p = OmegaCommon::FS::Path(opts.tempDir).append(_decl->name).absPath() + file_ext;
                    std::cout << p << std::endl;

                    shaderOut.open(p);

                    OmegaCommon::Vector<OmegaCommon::String> all_used_structs;
                    typeResolver->getStructsInFuncDecl(_decl,all_used_structs);
                    for(auto & s : all_used_structs){
                        shaderOut << generatedStructs[s] << std::endl << std::endl;
                    }

                    std::ostringstream extra_stmts;

                    indentLevel += 1;

                    /// @brief Write Each Standard Shader Argument
                    for(unsigned arg_idx = 0; arg_idx < _decl->params.size();arg_idx++){
                        auto &arg = _decl->params[arg_idx];
                        if(arg.attributeName.has_value()){
                            if(arg.attributeName.value() == ATTRIBUTE_VERTEX_ID){
                                for(unsigned i = 0;i < indentLevel;i++){
                                    extra_stmts << "  ";
                                }
                                writeTypeExpr(arg.typeExpr,extra_stmts);
                                extra_stmts << " " << arg.name << " = " << GLSL_VERTEX_ID << ";" << std::endl;
                            }
                        }
                        else {
                            shaderOut << "layout(location =";
                            shaderOut << arg_idx << ") in ";
                            writeTypeExpr(arg.typeExpr,shaderOut);
                            shaderOut << " " << arg.name << " ;" << std::endl;
                        }
                    }

                    /// @brief Write Shader Main Function along with extra data from attributed args.
                    shaderOut << "void main(){" << std::endl;

                    shaderOut << extra_stmts.str() << std::endl;

                    for(auto stmt_it = _decl->block->body.begin();stmt_it != _decl->block->body.end();stmt_it++){
                        auto stmt = *stmt_it;
                        for(unsigned i = 0;i < indentLevel;i++){
                            shaderOut << "  ";
                        }
                        if(stmt->type & DECL){
                            if(stmt->type == RETURN_DECL){
                                auto _return_stmt = (ast::ReturnDecl *)stmt;
                                shaderOut << return_val_replacement << " = ";
                                generateExpr(_return_stmt->expr);
                            }
                            else {
                                generateDecl((ast::Decl *)stmt);
                            }
                        }
                        else {
                            generateExpr((ast::Expr *)stmt);
                        }
                        shaderOut << ";" << std::endl; 
                    }
                    indentLevel -= 1;
                    shaderOut << "}" << std::endl;
                    shaderOut.close();
                    break;
                }
            }
        }
        void generateExpr(ast::Expr *expr) override{
            switch (expr->type) {
                case ID_EXPR : {
                    auto _expr = (ast::IdExpr *)expr;
                    shaderOut << _expr->id;
                    break;
                }
            }
        }
        void compileShader(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &name, const OmegaCommon::FS::Path &path, const OmegaCommon::FS::Path &outputPath) override {
            
        }
        void compileShaderOnRuntime(ast::ShaderDecl::Type type, const OmegaCommon::StrRef &source, const OmegaCommon::StrRef &name) override {
            #ifdef TARGET_VULKAN
                shaderc_shader_kind shader_kind;
                switch (type) {
                    case ast::ShaderDecl::Vertex : {
                        shader_kind = shaderc_glsl_vertex_shader;
                        break;
                    }
                    case ast::ShaderDecl::Fragment : {
                        shader_kind = shaderc_glsl_fragment_shader;
                        break;
                    }
                    case ast::ShaderDecl::Compute : {
                        shader_kind = shaderc_glsl_compute_shader;
                        break;
                    }
                }
                auto options = shaderc_compile_options_initialize();
                
                auto result = shaderc_compile_into_spv(compiler,source.data(), source.size(),shader_kind,name.data(),"main",options);
                auto & shader_entry = shaderMap[name.data()];
                shader_entry.data = (void *)(shaderc_result_get_bytes(result));
                shader_entry.dataSize = shaderc_result_get_length(result);

                shaderc_compile_options_release(options);

                shaderc_result_release(result);

            #endif
        }
        ~GLSLCodeGen(){
            shaderc_compiler_release(compiler);
        }
    };

    std::shared_ptr<CodeGen> GLSLCodeGenMake(CodeGenOpts & opts,GLSLCodeOpts &glslCodeOpts){
        return std::make_shared<GLSLCodeGen>(opts,glslCodeOpts);
    }
}