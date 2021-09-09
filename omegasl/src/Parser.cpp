#include "Parser.h"
#include "CodeGen.h"


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

        OmegaCommon::Map<ast::FuncDecl *,OmegaCommon::Vector<OmegaCommon::String>> funcDeclContextTypeMap;

        OmegaCommon::Vector<OmegaCommon::String> shaders;

        OmegaCommon::Vector<OmegaCommon::String> funcs;
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

            ast::builtins::uint_type}),currentContext(nullptr){

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

        void addTypeToCurrentContext(OmegaCommon::StrRef name, ast::Scope *loc){
            currentContext->typeMap.push_back(new ast::Type {name,loc,false});
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
                if(t->name == expr->name){
                    return t;
                }
            }

            auto type_it = currentContext->typeMap.begin();
            for(;type_it != currentContext->typeMap.end();type_it++){
                auto & t = *type_it;
                if(t->name == expr->name){
                    return t;
                }
            }

            return nullptr;
        };

        ast::TypeExpr * performSemForStmt(ast::Stmt *stmt){
            ast::TypeExpr *returnType = ast::TypeExpr::Create(KW_TY_VOID);

            return returnType;
        }

        ast::TypeExpr * performSemForBlock(ast::Block &block){

            std::vector<ast::TypeExpr *> allTypes;

            /// Assume void return if block is empty.
            if(block.body.empty()){
                allTypes.push_back(ast::TypeExpr::Create(KW_TY_VOID));
            }
            else {
                for(auto s : block.body){
                    allTypes.push_back(performSemForStmt(s));
                }
            }
            /// Pick first type to check with all others.
            auto picked_type = allTypes.front();
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

                    }

                    /// 3. If all of the above checks succeed, add struct type to TypeMap.
                    addTypeToCurrentContext(_decl->name,_decl->scope);
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

                    /// 2. Check shader params.
                    /// TODO: Add param uniqueness check
                    for(auto & p : _decl->params){
                        auto p_type = resolveTypeWithExpr(p.typeExpr);
                        if(p_type == nullptr){
                            return false;
                        }
                        if(p.attributeName.has_value()){
                            if(p.name == ATTRIBUTE_VERTEX_ID){
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
                    }

                    /// 3. Check function block.
                    auto eval_result = performSemForBlock(*_decl->block);

                    if(eval_result == nullptr){
                        return false;
                    }

                    /// 4. Check return types.
                    if(!_decl->returnType->compare(eval_result)){
                        std::cout << "In Function Return Type: Failed to match type." << std::endl;
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

    ast::TypeExpr *Parser::buildTypeRef(Tok &first_tok) {

        if(first_tok.type == TOK_ID || first_tok.type  == TOK_KW_TYPE){
            return ast::TypeExpr::Create(first_tok.str);
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
                    return nullptr;
                }

                t = lexer->nextTok();
                if(t.type != TOK_ID){
                    /// Error Expected Keyword
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
                        auto var_ty = buildTypeRef(t);
                        if(!var_ty){
                            return nullptr;
                        }
                        t = lexer->nextTok();
                        if(t.type != TOK_ID){
                            /// ERROR!
                            std::cout << "Expected ID" << std::endl;
                            return nullptr;
                        }
                        auto var_id = t.str;
                        t = lexer->nextTok();
                        if(t.type == TOK_COLON){
                            t = lexer->nextTok();
                            if(t.type != TOK_ID){
                                /// ERROR!
                                std::cout << "Expected ID" << std::endl;
                                return nullptr;
                            }
                            _decl->fields.push_back({var_ty,var_id,t.str});
                            t = lexer->nextTok();
                        }
                        else {
                            _decl->fields.push_back({var_ty,var_id,nullptr});
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
                    }
                    else {
                        _decl->scope = ast::builtins::global_scope;
                        return _decl;
                    }
                }
                else {
                    /// Error Unexpected Token
                    std::cout << "Unexpected Token" << std::endl;
                }
            }
            else if(t.str == KW_STRUCT) {
                // Error . Struct cannot have a resource map.
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
                std::cout << "Unexpected Keyword" << std::endl;
            }
            t = lexer->nextTok();
        }


        auto ty_for_decl = buildTypeRef(t);

        t = lexer->nextTok();
        if(t.type != TOK_ID){
            /// ERROR!
            std::cout << "Expected ID" << std::endl;
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
                auto var_ty = buildTypeRef(t);
                t = lexer->nextTok();
                if(t.type != TOK_ID){
                    /// ERROR!
                    std::cout << "Expected ID" << std::endl;
                    return nullptr;
                }

                auto var_id = t.str;

                t = lexer->nextTok();
                if(t.type == TOK_COLON){
                    t = lexer->nextTok();
                    if(t.type != TOK_ID){
                        // Error!
                        std::cout << "Expected ID" << std::endl;
                        return nullptr;
                    }

                    funcDecl->params.push_back({var_ty,var_id,t.str});
                    t = lexer->nextTok();
                }
                else {
                    funcDecl->params.push_back({var_ty,var_id,nullptr});
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
            node = resourceDecl;
        }
        node->scope = ast::builtins::global_scope;
        return node;
    }

    Tok &Parser::getTok() {
        return tokenBuffer[tokIdx++];
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


        first_tok = getTok();

        if(first_tok.type == TOK_KW){
            stmt = parseGenericDecl(first_tok,ctxt);
        }
        else {
            stmt = parseExpr(first_tok,ctxt.parentScope);
        }

        tokenBuffer.clear();
        return stmt;
    }

    ast::Decl *Parser::parseGenericDecl(Tok &first_tok,BlockParseContext & ctxt) {
        ast::Decl *node;
        if(first_tok.str == KW_IF){

        }
        else if(first_tok.str == KW_ELSE){

        }
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

                    ast::Expr *_child_expr = nullptr;
                   defaultR = parseOpExpr(first_tok,&_child_expr,parentScope);
                   if(!defaultR){
                       return defaultR;
                       break;
                   }

                   _call_expr->args.push_back(_child_expr);

                   first_tok = getTok();
                   if(first_tok.type == TOK_COMMA){
                       first_tok = getTok();
                   }
                }

                _expr = _call_expr;
            }
            else {
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
        ast::Expr *expr;
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
            auto stmt = parseStmt(first_tok,ctxt);
            if(!stmt){
                return nullptr;
            }
            block->body.push_back(stmt);
        }
        return block;
    }

    void Parser::parseContext(const ParseContext &ctxt) {

        auto semContext = std::make_shared<SemContext>();

        sem->setSemContext(semContext);

        lexer->setInputStream(&ctxt.in);
        ast::Decl *decl;
        while((decl = parseGlobalDecl()) != nullptr){
            if(sem->performSemForGlobalDecl(decl)){
                gen->generateDecl(decl);
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