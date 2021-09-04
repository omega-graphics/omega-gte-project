#include "Parser.h"
#include "CodeGen.h"

namespace omegasl {

    struct SemContext {
        OmegaCommon::Vector<ast::Type *> typeMap;
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
            ast::builtins::float_type,
            ast::builtins::uint_type}),currentContext(nullptr){

        };

        void getStructsInShaderDecl(ast::ShaderDecl *shaderDecl, std::vector<ast::StructDecl *> &out) override {

        }

        void setSemContext(std::shared_ptr<SemContext> & _currentContext){
            currentContext = _currentContext;
        }

        void addTypeToCurrentContext(OmegaCommon::StrRef name, ast::Scope *loc){
            currentContext->typeMap.push_back(new ast::Type {name,loc});
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
            return nullptr;
        }

        /// Parse Resource Map For ShaderDecl
        if(t.type == TOK_LBRACKET){
            auto _n = new ast::ShaderDecl();
            _n->type = SHADER_DECL;

            while((t = lexer->nextTok()).type != TOK_RBRACKET){
                if(t.type != TOK_KW){
                    // error
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
                }

                t = lexer->nextTok();
                if(t.type != TOK_ID){
                    /// Error Expected Keyword
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

            }
            else if(t.str == KW_STRUCT) {
                // Error . Struct cannot have resource properties.
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
            }

        }


        auto ty_for_decl = buildTypeRef(t);

        t = lexer->nextTok();
        if(t.type != TOK_ID){
            /// ERROR!
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

    ast::Stmt *Parser::parseStmt(Tok &first_tok) {

        ast::Stmt *stmt;
        tokIdx = 0;

        /// Get Token Line
        while(first_tok.type != TOK_SEMICOLON){
            tokenBuffer.push_back(first_tok);
            first_tok = lexer->nextTok();
        }


        first_tok = getTok();

        if(first_tok.type == TOK_KW){
            stmt = parseGenericDecl(first_tok);
        }
        else {

        }

        tokenBuffer.clear();
        return stmt;
    }

    ast::Decl *Parser::parseGenericDecl(Tok &first_tok) {
        ast::Decl *node;
        if(first_tok.str == KW_IF){

        }
        else if(first_tok.str == KW_ELSE){

        }
        return node;
    }

    bool Parser::parseObjectExpr(Tok &first_tok, ast::Expr **expr) {
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

        return defaultR;
    }

    bool Parser::parseArgsExpr(Tok &first_tok, Tok &second_tok, ast::Expr **expr) {
        bool defaultR = true;
        return defaultR;
    }

    bool Parser::parseOpExpr(Tok &first_tok, Tok &second_tok, ast::Expr **expr) {
        bool defaultR = true;
        return defaultR;
    }

    ast::Expr *Parser::parseExpr(Tok &first_tok,Tok &second_tok) {

        if(first_tok.type == TOK_LPAREN){
            auto t = lexer->nextTok();
            parseExpr(second_tok,t);
            if(t.type != TOK_RPAREN){
                // Throw Error.
            }
        }
        else {
            ast::Expr *expr;
            auto b = parseOpExpr(first_tok,second_tok,&expr);
            if(!b){
                return nullptr;
            }
            return expr;
        }
    }

    void Parser::parseContext(const ParseContext &ctxt) {

        auto semContext = std::make_shared<SemContext>();

        sem->setSemContext(semContext);

        lexer->setInputStream(&ctxt.in);
        ast::Decl *decl;
        while((decl = parseGlobalDecl()) != nullptr){
            gen->generateDecl(decl);
        }
        lexer->finishTokenizeFromStream();


    }

    ast::Block *Parser::parseBlock(Tok & first_tok) {
        /// First_Tok is equal to LBracket;
        auto *block = new ast::Block();
        while((first_tok = lexer->nextTok()).type != TOK_RBRACKET){
            auto stmt = parseStmt(first_tok);
            if(!stmt){
                return nullptr;
            }
            block->body.push_back(stmt);
        }
        return block;
    }

    Parser::~Parser() = default;



}