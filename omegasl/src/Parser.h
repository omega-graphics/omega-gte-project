#include "Lexer.h"
#include "AST.h"

#ifndef OMEGASL_PARSER_H
#define OMEGASL_PARSER_H

namespace omegasl {

    struct CodeGen;

    struct ParseContext {
        std::istream & in;
    };

    class Sem;

    class Parser {
        std::unique_ptr<Sem> sem;
        std::unique_ptr<Lexer> lexer;
        std::shared_ptr<CodeGen> &gen;

        std::vector<Tok> tokenBuffer;

        unsigned tokIdx;

        Tok & getTok();
        Tok & aheadTok();


        ast::Block *parseBlock(Tok & first_tok);

        bool parseObjectExpr(Tok &first_tok,ast::Expr **expr);
        bool parseArgsExpr(Tok &first_tok,Tok & second_tok,ast::Expr **expr);
        bool parseOpExpr(Tok &first_tok,Tok & second_tok,ast::Expr **expr);

        ast::Expr *parseExpr(Tok &first_tok,Tok &second_tok);
        ast::Decl *parseGenericDecl(Tok &first_tok);
        ast::Stmt *parseStmt(Tok &first_tok);
        ast::Decl *parseGlobalDecl();

        ast::TypeExpr *buildTypeRef(Tok &first_tok);
    public:
        explicit Parser(std::shared_ptr<CodeGen> &gen);
        void parseContext(const ParseContext &ctxt);
        ~Parser();
    };
}

#endif