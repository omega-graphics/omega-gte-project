#include <fstream>
#include <vector>

#ifndef OMEGASL_OMEGASLSYNTAX_H
#define OMEGASL_OMEGASLSYNTAX_H

namespace OmegaSL {

    #define TOKEN_MAX_SIZE 200

    struct FileLoc {
        unsigned startCol;
        unsigned endCol;
        unsigned line;
    };
    
    struct Tok {
        typedef enum : int {
            AtKeyword,
            Keyword,
            Identifier,
            OpenBrace,
            CloseBrace,
            Unknown
        } TokTy;
        TokTy type;
        FileLoc loc;
        std::string content;
    };


    struct AST {
        typedef enum {
            // Decls
            VertexShaderDecl,
            FragmentShaderDecl,
            ComputeShaderDecl,
            VarDecl,
            StructDecl,

            // Expr
            MemberExpr,
            PtrMemberExpr,
            ArrayExpr
        } NodeTy;
        struct Node {
            NodeTy type;
            void *params;
        };
        
    };


    class Syntax {
        std::vector<Tok> * tok_vector;
        class ExprBuilder {

        };
        std::unique_ptr<ExprBuilder> expr_builder;
    public:
        void setTokenVector(std::vector<Tok> *tok_vector);
        void convertToAST();
    };

    class Lexer {
        std::istream *in;
        char tokenBuffer[TOKEN_MAX_SIZE];
    public:
        char nextChar();
        char aheadChar();
        void setInputStream(std::istream *in);
        void tokenize(std::vector<Tok> & token_vector);
        Lexer();
    };
};

#endif