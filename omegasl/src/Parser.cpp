#include "Parser.h"
#include "CodeGen.h"

namespace std {
    template<>
    struct equal_to<omegasl::ast::ResourceDecl *> {
        auto operator()(omegasl::ast::ResourceDecl *lhs,omegasl::ast::ResourceDecl *rhs){
            return rhs->name == lhs->name;
        }
    };
}

namespace omegasl {

    enum class AttributeContext : int {
        VertexShaderArgument,
        FragmentShaderArgument,
        ComputeShaderArgument,
        StructField
    };

    inline bool isValidAttributeInContext(OmegaCommon::StrRef subject,AttributeContext context){
        return (subject == ATTRIBUTE_VERTEX_ID) || (subject == ATTRIBUTE_COLOR) || (subject == ATTRIBUTE_POSITION);
    }

    struct SemContext {
        OmegaCommon::Vector<ast::Type *> typeMap;

        OmegaCommon::SetVector<ast::ResourceDecl *> resourceSet;

        OmegaCommon::Map<ast::FuncDecl *,OmegaCommon::Vector<OmegaCommon::String>> funcDeclContextTypeMap;

        OmegaCommon::Vector<OmegaCommon::String> shaders;

        OmegaCommon::Vector<OmegaCommon::String> funcs;

        OmegaCommon::Map<OmegaCommon::String,ast::TypeExpr *> variableMap;
    };

    /// @brief Impl of Semantics Provider.
    /// @paragraph CodeGen communicates with this class for type data and TypeExpr evalutation.
    class Sem : public ast::SemFrontend {

        OmegaCommon::Vector<ast::Type *> builtinsTypeMap;

        std::shared_ptr<SemContext> currentContext;

    public:
        explicit Sem():
        builtinsTypeMap({
            ast::builtins::int_type,
            ast::builtins::uint_type,

            ast::builtins::float_type,
            ast::builtins::float2_type,
            ast::builtins::float3_type,
            ast::builtins::float4_type,

            ast::builtins::buffer_type,
            ast::builtins::texture1d_type
            }),currentContext(nullptr){

        };

        void getStructsInFuncDecl(ast::FuncDecl *funcDecl, std::vector<std::string> &out) override {
            for(auto & t : currentContext->funcDeclContextTypeMap){
                if(t.first == funcDecl){
                    out = t.second;
                }
            }
        }

        void setSemContext(std::shared_ptr<SemContext> & _currentContext){
            currentContext = _currentContext;
        }

        void addTypeToCurrentContext(OmegaCommon::StrRef name, ast::Scope *loc,OmegaCommon::Map<OmegaCommon::String,ast::TypeExpr *> & fields){
            currentContext->typeMap.push_back(new ast::Type {name,loc,false,{},fields});
        }

        bool hasTypeNameInFuncDeclContext(OmegaCommon::StrRef name,ast::FuncDecl *funcDecl){
            auto decl_res = currentContext->funcDeclContextTypeMap.find(funcDecl);

            auto name_it = decl_res->second.begin();
            for(;name_it != decl_res->second.end();name_it++){
                if(name == *name_it){
                    break;
                }
            }
            return name_it != decl_res->second.end();
        }

        void addTypeNameToFuncDeclContext(OmegaCommon::StrRef name,ast::FuncDecl *funcDecl){
            auto type_map_context = currentContext->funcDeclContextTypeMap.find(funcDecl);
            if(type_map_context == currentContext->funcDeclContextTypeMap.end()){
                currentContext->funcDeclContextTypeMap.insert(std::make_pair(funcDecl,OmegaCommon::Vector<OmegaCommon::String>{name}));
            }
            else {
                type_map_context->second.push_back(name);
            }

        }

        ast::Type * resolveTypeWithExpr(ast::TypeExpr *expr) override {

            auto b_type_it = builtinsTypeMap.begin();
            for(;b_type_it != builtinsTypeMap.end();b_type_it++){
                auto & t = *b_type_it;
//                std::cout << "BUILTIN:" << t->name << " COMPARE TO:" << expr->name << std::endl;
                if(t->name == expr->name){
                    return t;
                }
            }

            auto type_it = currentContext->typeMap.begin();
            for(;type_it != currentContext->typeMap.end();type_it++){
                auto & t = *type_it;
//                std::cout << "Struct Type:" << t->name << std::endl;
                if(t->name == expr->name){
                    return t;
                }
            }

//            std::cout << "Unknown Type:" << expr->name << std::endl;

            return nullptr;
        };

        ast::TypeExpr *performSemForDecl(ast::Decl * decl,ast::FuncDecl *funcContext){
            auto ret = ast::TypeExpr::Create(KW_TY_VOID);
            switch (decl->type) {
                case VAR_DECL : {
                    auto _decl = (ast::VarDecl *)decl;
                    auto type_res = resolveTypeWithExpr(_decl->typeExpr);
                    if(type_res == nullptr){
                        return nullptr;
                    }

                    OmegaCommon::StrRef t_name = type_res->name;

                    if(!type_res->builtin) {
                        if(!hasTypeNameInFuncDeclContext(t_name,funcContext)){
                            addTypeNameToFuncDeclContext(t_name,funcContext);
                        }
                    }

                    currentContext->variableMap.insert(std::make_pair(_decl->spec.name,_decl->typeExpr));
                    break;
                }
                case RETURN_DECL : {
                    auto _decl = (ast::ReturnDecl *)decl;
                    return performSemForExpr(_decl->expr,funcContext);
                }
            }
            return ret;
        }

        ast::TypeExpr *performSemForExpr(ast::Expr * expr,ast::FuncDecl *funcContext){
            auto ret = ast::TypeExpr::Create(KW_TY_VOID);
            if(expr->type == ID_EXPR){
                auto _expr = (ast::IdExpr *)expr;
                auto _id_found = currentContext->variableMap.find(_expr->id);
                if(_id_found == currentContext->variableMap.end()){
                    std::cout << "Unknown Identifier: " << _expr->id << std::endl;
                    return nullptr;
                }
                else {
                    return _id_found->second;
                }
            }
            else if(expr->type == LITERAL_EXPR){
                auto _expr = (ast::LiteralExpr *)expr;
                if(_expr->isInt()){
                    return ast::TypeExpr::Create(ast::builtins::int_type);
                }
                else if(_expr->isUint()){
                    return ast::TypeExpr::Create(ast::builtins::uint_type);
                }
                else if(_expr->isFloat()){
                    return ast::TypeExpr::Create(ast::builtins::float_type);
                }
            }
            else if(expr->type == ARRAY_EXPR){
                auto _expr = (ast::ArrayExpr *)expr;
            }
            else if(expr->type == MEMBER_EXPR){
                auto _expr = (ast::MemberExpr *)expr;
                auto t = performSemForExpr(_expr->lhs,funcContext);

                std::cout << "MEMBER EXPR " << t->name << " MEMBER:" << _expr->rhs_id << std::endl;

                if(t == nullptr){
                    return nullptr;
                }

                auto type_res = resolveTypeWithExpr(t);
                if(!type_res->builtin){
                    auto member_found = type_res->fields.find(_expr->rhs_id);
                    if(member_found == type_res->fields.end()){
                        std::cout << "Member `" << _expr->rhs_id << "` does not exist on struct " << type_res->name << std::endl;
                        return nullptr;
                    }
                    else {
                        return member_found->second;
                    }
                }
            }
            else if(expr->type == BINARY_EXPR){
                auto _expr = (ast::BinaryExpr *)expr;
                auto rhs_res = performSemForExpr(_expr->rhs,funcContext);
                if(!rhs_res){
                    return nullptr;
                }

                auto lhs_res = performSemForExpr(_expr->lhs,funcContext);
                if(!lhs_res){
                    return nullptr;
                }

                if(!rhs_res->compare(lhs_res)){
                    std::cout << "Failed to match type in binary expr." << std::endl;
                    return nullptr;
                }
            }
            return ret;
        }

        ast::TypeExpr * performSemForStmt(ast::Stmt *stmt,ast::FuncDecl *funcContext){
            ast::TypeExpr *returnType = nullptr;

            if(stmt->type & DECL){
                returnType = performSemForDecl((ast::Decl *)stmt,funcContext);
            }
            else {
                returnType = performSemForExpr((ast::Expr *)stmt,funcContext);
            }

            return returnType;
        }

        ast::TypeExpr * performSemForBlock(ast::Block &block,ast::FuncDecl *funcContext){

            std::vector<ast::TypeExpr *> allTypes;

            /// Assume void return if block is empty.
            if(block.body.empty()){
                allTypes.push_back(ast::TypeExpr::Create(KW_TY_VOID));
            }
            else {
                for(auto s : block.body){
                    auto res = performSemForStmt(s,funcContext);
                    if(!res){
                        std::cout << "Failed to perform sem on Block stmt" << std::endl;
                        return nullptr;
                    }

                    if(s->type == RETURN_DECL){
                        allTypes.push_back(res);
                    }
                }
            }
            /// Pick first type to check with all others.
            auto picked_type = allTypes.front();
            std::cout << "Picked Type:" << picked_type->name << std::endl;
            if(allTypes.size() > 1) {
                for(auto t_it = allTypes.begin() + 1;t_it != allTypes.end();++t_it){
                    if(!picked_type->compare(*t_it)){
                        std::cout << "In function return Context" << std::endl;
                        return nullptr;
                    }
                }
            }
            return picked_type;
        }

        bool performSemForGlobalDecl(ast::Decl *decl){
            switch (decl->type) {
                case STRUCT_DECL : {
                    auto *_decl = (ast::StructDecl *)decl;
                    /// 1. Check TypeMap if type is defined with name already
                    ast::TypeExpr *test_expr = ast::TypeExpr::Create(_decl->name);
                    auto res = resolveTypeWithExpr(test_expr);
                    if(res != nullptr){
                        std::cout << _decl->name << " already is defined as a type." << std::endl;
                        delete test_expr;
                        return false;
                    }
                    delete test_expr;

                    bool & isInternal = _decl->internal;
                    /// 2. Check struct fields.
                    /// TODO: Add struct field uniqueness check

                    OmegaCommon::Map<OmegaCommon::String,ast::TypeExpr *> field_types;

                    for(auto & f : _decl->fields){

                        auto field_ty = resolveTypeWithExpr(f.typeExpr);
                        if(field_ty == nullptr){
                            return false;
                        }

                        if(f.attributeName.has_value()){
                            if(!isInternal){
                                std::cout << "In struct" << f.name << std::endl << "Cannot use attributes on fields in public structs" << std::endl;
                                return false;
                            }

                            if(!isValidAttributeInContext(f.attributeName.value(),AttributeContext::StructField)){
                                std::cout << "In struct" << f.name << std::endl << "Invalid attribute name `" << f.attributeName.value() << "` " << std::endl;
                                return false;
                            }
                        }

                        field_types.insert(std::make_pair(f.name,f.typeExpr));

                    }

                    /// 3. If all of the above checks succeed, add struct type to TypeMap.
                    std::cout << "Adding Struct Type:" << _decl->name << std::endl;
                    addTypeToCurrentContext(_decl->name,_decl->scope,field_types);
                    break;
                }
                case RESOURCE_DECL : {

                    /// 1. Check is resource name is taken
                    auto *_decl = (ast::ResourceDecl *)decl;
                    if(currentContext->resourceSet.find(_decl) == currentContext->resourceSet.end()){
                        currentContext->resourceSet.push(_decl);
                    }
                    else {
                        std::cout << "Resource " << _decl->name << " has already been declared!" << std::endl;
                        return false;
                    }

                    /// 2. Check resource type.
                    auto ty = resolveTypeWithExpr(_decl->typeExpr);
                    if(ty != ast::builtins::buffer_type && ty != ast::builtins::texture1d_type){
                        std::cout << "Resource `" << _decl->name << "` is not a valid type. (" << _decl->typeExpr->name << ")" << std::endl;
                        return false;
                    }


                    break;
                }
                case SHADER_DECL : {
                    auto *_decl = (ast::ShaderDecl *)decl;
                    /// 1. Check if shader has been declared.

                    auto findShader = [&](){
                        bool f = false;
                        for(OmegaCommon::StrRef s : currentContext->shaders){
                            if(s == _decl->name){
                                f = true;
                                break;
                            }
                        }
                        return f;
                    };

                    auto found = findShader();
                    if(found){
                        std::cout << "Shader " << _decl->name << " has already been declared." << std::endl;
                        return false;
                    }

                    auto & shaderType = _decl->shaderType;

                    /// 2. Check shader params and pipeline layout.
                    /// TODO: Add param uniqueness check

                    for(auto & r : _decl->resourceMap){
                        for(auto res : currentContext->resourceSet){
                            if(res->name == r.name){
                                currentContext->variableMap.insert(std::make_pair(r.name,res->typeExpr));
                            }
                        }
                    }

                    for(auto & p : _decl->params){
                        auto p_type = resolveTypeWithExpr(p.typeExpr);
                        if(p_type == nullptr){
                            return false;
                        }
                        /// Add struct type if param is an internal struct.
                        if(!p_type->builtin){
                            if(!hasTypeNameInFuncDeclContext(p_type->name,_decl)){
                                addTypeNameToFuncDeclContext(p_type->name,_decl);
                            }
                        }

                        if(p.attributeName.has_value()){
                            if(p.attributeName == ATTRIBUTE_VERTEX_ID){
                                if(shaderType != ast::ShaderDecl::Vertex){
                                    std::cout << "Cannot use " << ATTRIBUTE_VERTEX_ID << " in this context." << std::endl;
                                    return false;
                                }
                                else if(_decl->params.size() > 1){
                                    std::cout << "Cannot use " << ATTRIBUTE_VERTEX_ID << " in this context. (When using this attribute on a parameter, there may be only one parameter for the parent function.)" << std::endl;
                                    return false;
                                }
                            }
                        }
                        currentContext->variableMap.insert(std::make_pair(p.name,p.typeExpr));
                    }

                    /// 3. Check function block.
                    auto eval_result = performSemForBlock(*_decl->block,_decl);

                    if(eval_result == nullptr){
                        return false;
                    }

                    /// 4. Check return types.
                    if(!_decl->returnType->compare(eval_result)){
                        std::cout << "In Function Return Type: Failed to match type." << "(" << _decl->returnType->name << " vs. " << eval_result->name << ")" << std::endl;
                        return false;
                    }

                    /// 5. Check returntype is struct type and add struct to shader context;
                    auto t = resolveTypeWithExpr(eval_result);
                    if(!t->builtin){
                        if(!hasTypeNameInFuncDeclContext(t->name,_decl)){
                            addTypeNameToFuncDeclContext(t->name,_decl);
                        }
                    }


                    /// 6. Add shader to context
                    currentContext->shaders.push_back(_decl->name);
                    /// 7. Clear Variable map.
                    currentContext->variableMap.clear();
                    break;
                }
                default : {
                    std::cout << "Cannot declare ast::Stmt of type:" << std::hex << decl->type << std::dec << std::endl;
                    return false;
                }
            }
            return true;
        }
    };

    Parser::Parser(std::shared_ptr<CodeGen> &gen):
    lexer(std::make_unique<Lexer>()),
    gen(gen),
    sem(std::make_unique<Sem>()),tokIdx(0){
        gen->setTypeResolver(sem.get());
    }

    ast::TypeExpr *Parser::buildTypeRef(Tok &first_tok,bool isPointer,bool hasTypeArgs,OmegaCommon::Vector<ast::TypeExpr *> * args) {

        if(first_tok.type == TOK_ID || first_tok.type  == TOK_KW_TYPE){
            return ast::TypeExpr::Create(first_tok.str,isPointer,hasTypeArgs,args);
        }
        else {
            // ERROR
            return nullptr;
        }
    }

    ast::Decl *Parser::parseGlobalDecl() {
        ast::Decl *node = nullptr;
        bool shaderDecl;

        auto t = lexer->nextTok();
        if(t.type == TOK_EOF){
            std::cout << "EOF" << std::endl;
            return nullptr;
        }

        /// Parse Resource Map For ShaderDecl
        if(t.type == TOK_LBRACKET){
            auto _n = new ast::ShaderDecl();
            _n->type = SHADER_DECL;

            while((t = lexer->nextTok()).type != TOK_RBRACKET){
                if(t.type != TOK_KW){
                    // error
                    std::cout << "Expected KW" << std::endl;
                    return nullptr;
                }

                ast::ShaderDecl::ResourceMapDesc mapDesc;

                if(t.str == KW_IN){
                    mapDesc.access = ast::ShaderDecl::ResourceMapDesc::In;
                }
                else if(t.str == KW_INOUT){
                    mapDesc.access = ast::ShaderDecl::ResourceMapDesc::Inout;
                }
                else if(t.str == KW_OUT){
                    mapDesc.access = ast::ShaderDecl::ResourceMapDesc::Out;
                }
                else {
                    /// Error (Unexpected Keyword)
                    std::cout << "Unexpected KW" << std::endl;
                    return nullptr;
                }

                t = lexer->nextTok();
                if(t.type != TOK_ID){
                    /// Error Expected Keyword
                    std::cout << "Expected KW" << std::endl;
                    return nullptr;
                }
                mapDesc.name = t.str;
                _n->resourceMap.push_back(mapDesc);

            }

            node = (ast::Decl *)_n;
            t = lexer->nextTok();
        }

        if(t.type == TOK_KW){
            /// Parse Struct
            if(t.str == KW_STRUCT && node == nullptr){
                auto _decl = new ast::StructDecl();
                _decl->type = STRUCT_DECL;
                _decl->internal = false;
                t = lexer->nextTok();
                if(t.type != TOK_ID){
                    // Expected ID.
                }
                _decl->name = t.str;

                t = lexer->nextTok();

                if(t.type == TOK_KW){
                    if(t.str != KW_INTERNAL){
                        // Expected No Keyword or Internal
                        std::cout << "Expected Internal Keyword or No Keyword" << std::endl;
                        return nullptr;
                    }
                    _decl->internal = true;
                    t = lexer->nextTok();
                }

                if(t.type == TOK_LBRACE){
                    t = lexer->nextTok();
                    while(t.type != TOK_RBRACE){
                        auto _tok = t;
                        t = lexer->nextTok();
                        bool type_is_pointer = false;
                        if(t.type == TOK_ASTERISK){
                            type_is_pointer = true;
                            t = lexer->nextTok();
                        }

                        auto var_ty = buildTypeRef(_tok,type_is_pointer);
                        if(!var_ty){
                            return nullptr;
                        }

                        if(t.type != TOK_ID){
                            /// ERROR!
                            std::cout << "Expected ID. Instead got:" << t.str << std::endl;
                            return nullptr;
                        }
                        auto var_id = t.str;
                        t = lexer->nextTok();
                        if(t.type == TOK_COLON){
                            t = lexer->nextTok();
                            if(t.type != TOK_ID){
                                /// ERROR!
                                std::cout << "Expected ID. Instead got:" << t.str << std::endl;
                                return nullptr;
                            }
                            _decl->fields.push_back({var_ty,var_id,t.str});
                            t = lexer->nextTok();
                        }
                        else {
                            _decl->fields.push_back({var_ty,var_id,{}});
                        }

                        if(t.type != TOK_SEMICOLON){
                            /// Error. Expected Semicolon.
                            std::cout << "Expected SemiColon" << std::endl;
                            return nullptr;
                        }
                        t = lexer->nextTok();
                    }

                    t = lexer->nextTok();

                    if(t.type != TOK_SEMICOLON){
                        /// Error. Expected Semicolon.
                        std::cout << "Expected Semicolon" << std::endl;
                        return nullptr;
                    }
                    else {
                        _decl->scope = ast::builtins::global_scope;
                        return _decl;
                    }
                }
                else {
                    /// Error Unexpected Token
                    std::cout << "Unexpected Token" << std::endl;
                    return nullptr;
                }
            }
            else if(t.str == KW_STRUCT) {
                // Error . Struct cannot have a resource map.
                std::cout << "Struct cannot have a resource map!" << std::endl;
                return nullptr;
            }

            if(node == nullptr){
                node = (ast::Decl *)new ast::ShaderDecl();
                node->type = SHADER_DECL;
            }

            if(t.str == KW_VERTEX){
                auto _s = (ast::ShaderDecl *)node;
                _s->shaderType = ast::ShaderDecl::Vertex;
            }
            else if(t.str == KW_FRAGMENT){
                auto _s = (ast::ShaderDecl *)node;
                _s->shaderType = ast::ShaderDecl::Fragment;
            }
            else if(t.str == KW_COMPUTE){
                auto _s = (ast::ShaderDecl *)node;
                _s->shaderType = ast::ShaderDecl::Compute;
            }
            else {
                /// TODO: Error! Unexpected Keyword!
                delete node;
                std::cout << "Unexpected Keyword" << std::endl;
                return nullptr;
            }
            t = lexer->nextTok();
        }

        auto _tok = t;
        t = lexer->nextTok();
        OmegaCommon::Vector<ast::TypeExpr *> type_args;
        bool type_is_pointer = false,has_type_args = false;

        if(t.type == TOK_OP && t.str == OP_LESS){
            has_type_args = true;
            t = lexer->nextTok();
            while(t.type != TOK_OP && t.str != OP_GREATER){
                if(t.type == TOK_ID){
                    type_args.push_back(ast::TypeExpr::Create(t.str));
                }
                t = lexer->nextTok();
            }
            t = lexer->nextTok();
        }

        if(t.type == TOK_ASTERISK){
            type_is_pointer = true;
            t = lexer->nextTok();
        }

        auto ty_for_decl = buildTypeRef(_tok,type_is_pointer,has_type_args,&type_args);

        if(t.type != TOK_ID){
            /// ERROR!
            delete node;
            std::cout << "Expected ID. Instead got:" << t.str << std::endl;
            return nullptr;
        }

        auto id_for_decl = t.str;
        t = lexer->nextTok();

        /// Parse FuncDecl/ShaderDecl
        if(t.type == TOK_LPAREN){
            ast::FuncDecl *funcDecl;
            if(node == nullptr){
                funcDecl = new ast::FuncDecl();
                funcDecl->type = FUNC_DECL;
            }
            else {
                /// ShaderDecl is a FuncDecl!
                funcDecl = (ast::FuncDecl *)node;
            }

            funcDecl->name = id_for_decl;
            funcDecl->returnType = ty_for_decl;
            t = lexer->nextTok();

            while(t.type != TOK_RPAREN){
                auto _tok = t;
                t = lexer->nextTok();
                bool type_is_pointer = false;

                if(t.type == TOK_ASTERISK){
                    type_is_pointer = true;
                    t = lexer->nextTok();
                }

                auto var_ty = buildTypeRef(_tok,type_is_pointer);

                if(t.type != TOK_ID){
                    /// ERROR!
                    delete node;
                    std::cout << "Expected ID. Instead got:" << t.str << std::endl;
                    return nullptr;
                }

                auto var_id = t.str;

                t = lexer->nextTok();
                if(t.type == TOK_COLON){
                    t = lexer->nextTok();
                    if(t.type != TOK_ID){
                        // Error!
                        delete node;
                        std::cout << "Expected ID. Instead got:" << t.str << std::endl;
                        return nullptr;
                    }

                    funcDecl->params.push_back({var_ty,var_id,t.str});
                    t = lexer->nextTok();
                }
                else {
                    funcDecl->params.push_back({var_ty,var_id,{}});
                }

                if(t.type == TOK_COMMA){
                    t = lexer->nextTok();
                    if(t.type == TOK_RPAREN){
                        /// Error; Unexpected TOken.
                        std::cout << "Expected RParen" << std::endl;
                        return nullptr;
                    }
                }
            }

            t = lexer->nextTok();
            if(t.type != TOK_LBRACE){
                std::cout << "Expected Tok. Expected LBrace.";
                /// Error. Unexpected Token.
                return nullptr;
            }

            BlockParseContext blockParseContext {ast::builtins::global_scope,true};

            funcDecl->block.reset(parseBlock(t,blockParseContext));

            if(!funcDecl->block){
                return nullptr;
            }

        }
        else if(t.type == TOK_COLON){
            auto * resourceDecl = new ast::ResourceDecl();
            resourceDecl->type = RESOURCE_DECL;
            resourceDecl->name = id_for_decl;
            resourceDecl->typeExpr = ty_for_decl;
            t = lexer->nextTok();
            if(t.type != TOK_NUM_LITERAL){
                delete resourceDecl;
                std::cout << "Expected NUM Literal" << std::endl;
                return nullptr;
            }
            resourceDecl->registerNumber = std::stoul(t.str);
            t = lexer->nextTok();
            if(t.type != TOK_SEMICOLON){
                delete resourceDecl;
                std::cout << "Expected Semicolon" << std::endl;
            }
            node = resourceDecl;
        }
        node->scope = ast::builtins::global_scope;
        return node;
    }

    Tok &Parser::getTok() {
        return tokenBuffer[++tokIdx];
    }
    Tok & Parser::aheadTok() {
        return tokenBuffer[tokIdx + 1];
    }

    ast::Stmt *Parser::parseStmt(Tok &first_tok,BlockParseContext & ctxt) {

        ast::Stmt *stmt;
        tokIdx = 0;

        /// Get Entire Line of Tokens
        while(first_tok.type != TOK_SEMICOLON){
            tokenBuffer.push_back(first_tok);
            first_tok = lexer->nextTok();
        }

        tokenBuffer.push_back(first_tok);

        for(auto & t : tokenBuffer){
            std::cout << "TOK: {t:" << std::hex << t.type << std::dec << ",str:" << t.str << "}" << std::endl;
        }


        first_tok = tokenBuffer.front();

        bool isDecl = false;

        /// @note Checks for TypeRef!
        if(first_tok.type == TOK_KW_TYPE || first_tok.type == TOK_ID){
            auto & ahead_tok = aheadTok();

            if(ahead_tok.type == TOK_ASTERISK || ahead_tok.type == TOK_ID){
                isDecl = true;
            }
        }
        else if(first_tok.type == TOK_KW){
            isDecl = true;
        }

        if(isDecl){
            std::cout << "Parse Generic Decl" << std::endl;
            stmt = parseGenericDecl(first_tok,ctxt);
        }
        else {
            std::cout << "Parse Expr" << std::endl;
            stmt = parseExpr(first_tok,ctxt.parentScope);
        }

        tokenBuffer.clear();
        return stmt;
    }

    ast::Decl *Parser::parseGenericDecl(Tok &first_tok,BlockParseContext & ctxt) {
        ast::Decl *node = nullptr;
        if(first_tok.type == TOK_KW){
            if(first_tok.str == KW_RETURN){
                first_tok = getTok();
                auto _decl = new ast::ReturnDecl();
                _decl->type = RETURN_DECL;
                auto _e = parseExpr(first_tok,ctxt.parentScope);
                if(_e == nullptr){
                    delete _decl;
                    return nullptr;
                }
                _decl->expr = _e;
                node = _decl;
            }
        }
        else {
            /// @note Build TypeRef for VarDecl.
            auto _tok = first_tok;
            first_tok = aheadTok();
            bool type_is_pointer = false;
            if(first_tok.type == TOK_ASTERISK){
                type_is_pointer = true;
                ++tokIdx;
            }
            auto type_for_var_decl = buildTypeRef(_tok,type_is_pointer);
            first_tok = getTok();
            if(first_tok.type != TOK_ID){
                std::cout << "Expected ID!" << std::endl;
                return nullptr;
            }
            auto _decl = new ast::VarDecl();
            _decl->type = VAR_DECL;
            _decl->typeExpr = type_for_var_decl;
            _decl->spec.name = first_tok.str;
            first_tok = aheadTok();
            if(first_tok.type == TOK_OP && first_tok.str == OP_EQUAL){
                ++tokIdx;
                first_tok = getTok();
                auto _e = parseExpr(first_tok,ctxt.parentScope);
                if(_e == nullptr){
                    return nullptr;
                }
                _decl->spec.initializer = _e;
            }
            else if(first_tok.type == TOK_OP){
                std::cout << "Unknown Operator" << first_tok.str << std::endl;
                return nullptr;
            }
            node = _decl;
        }
        node->scope = ctxt.parentScope;
        return node;
    }

    bool Parser::parseObjectExpr(Tok &first_tok, ast::Expr **expr,ast::Scope *parentScope) {
        bool defaultR = true;
        if(first_tok.type == TOK_ID){
            auto _e = new ast::IdExpr();
            _e->type = ID_EXPR;
            _e->id = first_tok.str;
            *expr = _e;
        }
        else if(first_tok.type == TOK_STR_LITERAL){
            auto _e = new ast::LiteralExpr();
            _e->type = LITERAL_EXPR;
            _e->str = first_tok.str;
            *expr = _e;
        }
        else if(first_tok.type == TOK_NUM_LITERAL){
            auto _e = new ast::LiteralExpr();
            _e->type = LITERAL_EXPR;
            *expr = _e;
        }
        else if(first_tok.type == TOK_LBRACE){
            auto _e = new ast::ArrayExpr();
            _e->type = ARRAY_EXPR;
            while((first_tok = getTok()).type != TOK_RBRACE){
                auto _child_e = parseExpr(first_tok,parentScope);
                first_tok = aheadTok();
                if(first_tok.type == TOK_COMMA){
                    ++tokIdx;
                }
                _e->elm.push_back(_child_e);
            }
            *expr = _e;
        }
        else {
            std::cout << "Unexpected Token:" << first_tok.str << std::endl;
            return false;
        }
        (*expr)->scope = parentScope;

        return defaultR;
    }

    bool Parser::parseArgsExpr(Tok &first_tok, ast::Expr **expr,ast::Scope *parentScope) {
        bool defaultR = true;
        ast::Expr *_expr = nullptr;
        defaultR = parseObjectExpr(first_tok,&_expr,parentScope);
        first_tok = aheadTok();
        while(true){
            if(first_tok.type == TOK_LPAREN){
                ++tokIdx;

                auto * _call_expr = new ast::CallExpr();
                _call_expr->type = CALL_EXPR;
                _call_expr->callee = _expr;

                first_tok = getTok();
                while(first_tok.type != TOK_RPAREN){

                   auto _child_expr = parseExpr(first_tok,parentScope);
                   if(_child_expr == nullptr){
                       return false;
//                       break;
                   }

                   _call_expr->args.push_back(_child_expr);

                   first_tok = getTok();
                   if(first_tok.type == TOK_COMMA){
                       first_tok = getTok();
                   }
                }

                _expr = _call_expr;
            }
            else if(first_tok.type == TOK_DOT){
                ++tokIdx;

                auto _member_expr = new ast::MemberExpr();
                _member_expr->type = MEMBER_EXPR;
                _member_expr->lhs = _expr;

                first_tok = getTok();
                if(first_tok.type != TOK_ID){
                    std::cout <<"Expected ID" << std::endl;
                    *expr = nullptr;
                    delete _member_expr;
                    return false;
                }

                _member_expr->rhs_id = first_tok.str;
                _expr = _member_expr;
            }
            else {
                *expr = _expr;
                break;
            }
            first_tok = aheadTok();
            *expr = _expr;
        }


        return defaultR;
    }

    bool Parser::parseOpExpr(Tok &first_tok, ast::Expr **expr,ast::Scope *parentScope) {
        bool defaultR = true;
        bool hasPrefixOp = false;

        ast::Expr *_expr = nullptr;

        /// @note Unary Pre-Expr Operator Tokens!

        switch (first_tok.type) {
            case TOK_LPAREN : {
                first_tok = getTok();
                _expr = parseExpr(first_tok,parentScope);
                first_tok = getTok();
                if(first_tok.type != TOK_RPAREN){
                    std::cout << "Expected RParen!" << std::endl;
                    return false;
                }
                break;
            }
            case TOK_ASTERISK : {
                hasPrefixOp = true;
                auto _pointer_expr = new ast::PointerExpr();
                _pointer_expr->type = POINTER_EXPR;
                _pointer_expr->ptType = ast::PointerExpr::Dereference;

                first_tok = getTok();
                _expr = parseExpr(first_tok,parentScope);
                if(_expr == nullptr){
                    delete _pointer_expr;
                    return false;
                }

                _pointer_expr->expr = _expr;
                _expr = _pointer_expr;
                break;
            }
            case TOK_AMPERSAND : {
                hasPrefixOp = true;
                auto _pointer_expr = new ast::PointerExpr();
                _pointer_expr->type = POINTER_EXPR;
                _pointer_expr->ptType = ast::PointerExpr::AddressOf;

                first_tok = getTok();
                _expr = parseExpr(first_tok,parentScope);
                if(_expr == nullptr){
                    delete _pointer_expr;
                    return false;
                }

                _pointer_expr->expr = _expr;
                _expr = _pointer_expr;
                break;
            }
            /// Pre-Expr Operators
            case TOK_OP : {
                hasPrefixOp = true;
                OmegaCommon::StrRef op_type = first_tok.str;
                if(op_type != OP_NOT || op_type != OP_PLUSPLUS || op_type != OP_MINUSMINUS){
                    std::cout << "Invalid operator" << op_type << "in this context." << std::endl;
                    return false;
                }
                auto _unary_op_expr = new ast::UnaryOpExpr();
                _unary_op_expr->type = UNARY_EXPR;
                _unary_op_expr->isPrefix = true;
                _unary_op_expr->op = op_type;
                _unary_op_expr->scope = parentScope;

                first_tok = getTok();
                _expr = parseExpr(first_tok,parentScope);

                if(_expr == nullptr){
                    return false;
                }
                _unary_op_expr->expr = _expr;

                _expr = _unary_op_expr;
                break;
            }
            default : {
                defaultR = parseArgsExpr(first_tok,&_expr,parentScope);
                break;
            }
        }

        /// @note Post-Expr Operators (Unary or Binary)

        if(!hasPrefixOp) {

            first_tok = aheadTok();

            if(first_tok.type == TOK_OP){
                ++tokIdx;
                if(first_tok.str == OP_MINUSMINUS || first_tok.str == OP_PLUSPLUS){
                    auto unaryExpr = new ast::UnaryOpExpr();
                    unaryExpr->type = UNARY_EXPR;
                    unaryExpr->op = first_tok.str;
                    unaryExpr->isPrefix = false;
                    unaryExpr->expr = _expr;
                    _expr = unaryExpr;
                }
                else {
                    auto binaryExpr = new ast::BinaryExpr();
                    binaryExpr->type = BINARY_EXPR;
                    binaryExpr->op = first_tok.str;
                    binaryExpr->lhs = _expr;
                    first_tok = getTok();
                    _expr = parseExpr(first_tok,parentScope);
                    if(_expr == nullptr){
                        return false;
                    }
                    binaryExpr->rhs = _expr;
                    binaryExpr->scope = parentScope;
                    _expr = binaryExpr;
                }
            }

        }

        *expr = _expr;
        return defaultR;
    }

    ast::Expr *Parser::parseExpr(Tok &first_tok,ast::Scope *parentScope) {
        ast::Expr *expr = nullptr;
        auto b = parseOpExpr(first_tok,&expr,parentScope);
        if(!b){
            return nullptr;
        }
        return expr;
    }

    ast::Block *Parser::parseBlock(Tok & first_tok,BlockParseContext & ctxt) {
        /// First_Tok is equal to LBracket;
        auto *block = new ast::Block();
        while((first_tok = lexer->nextTok()).type != TOK_RBRACE){
            if(first_tok.type != TOK_SEMICOLON){
                auto stmt = parseStmt(first_tok,ctxt);
                if(!stmt){
                    std::cout << "Failed to parse block stmt" << std::endl;
                    delete block;
                    return nullptr;
                }
                block->body.push_back(stmt);
            }
        }
        return block;
    }

    void Parser::parseContext(const ParseContext &ctxt) {

        auto semContext = std::make_shared<SemContext>();

        sem->setSemContext(semContext);

        lexer->setInputStream(&ctxt.in);
        ast::Decl *decl;
        while((decl = parseGlobalDecl()) != nullptr){
            std::cout << std::hex << "NODE TYPE:" << decl->type << std::dec << std::endl;
            if(sem->performSemForGlobalDecl(decl)){
                gen->generateDecl(decl);
                gen->generateInterfaceAndCompileShader(decl);
            }
            else {
                std::cout << "Failed to evaluate stmt" << std::endl;
                break;
            }
        }
        lexer->finishTokenizeFromStream();

    }

    Parser::~Parser() = default;

}