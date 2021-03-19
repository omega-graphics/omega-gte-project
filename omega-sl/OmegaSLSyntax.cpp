#include "OmegaSLSyntax.h"
#include <cstdint>
#include <cctype>

namespace OmegaSL {
    #define KEYWORD(name) "name"

    #define ATVERTEX_KW KEYWORD(@vertex)
    #define ATFRAGMENT_KW KEYWORD(@fragment)
    #define ATCOMPUTE_KW KEYWORD(@compute)
    #define STRUCT_KW KEYWORD(struct)


    enum class Keyword : int {
        AtVertex,
        AtFragment,
        AtCompute,
        Struct,
        UnknownOrOther
    };

    bool isRegularKeyword(std::string & str){
       return str == STRUCT_KW;
    };

    
    void Lexer::setInputStream(std::istream *in){
        this->in = in;
    };
    static void pushTokenToVector(std::vector<Tok> & token_vector,char *buffer,uint64_t buffer_len,Tok::TokTy type,FileLoc & loc){
        Tok tok;
        tok.content = std::string(buffer,buffer_len);
        tok.loc = loc;
        tok.type = isRegularKeyword(tok.content)? Tok::Keyword : type;

        token_vector.push_back(std::move(tok));
    };
    char Lexer::nextChar(){
        return in->get();
    };
    char Lexer::aheadChar(){
        return in->peek();
    };

    void Lexer::tokenize(std::vector<Tok> &token_vector){
        char *bufferStart = tokenBuffer;
        char *bufferEnd = bufferStart;
        #define RESET bufferEnd = bufferStart
        char c = nextChar();
        Tok::TokTy type_estimate;
        FileLoc f_loc;
        f_loc.line = 1;
        f_loc.endCol = 0;
        while(!in->eof()){
            switch (c) {
                case '@':{
                    *bufferEnd = c;
                    ++bufferEnd;
                    type_estimate = Tok::AtKeyword;
                    break;
                }
                case ' '  | '\r' : {
                    c = aheadChar();
                    if(isalpha(c)){
                        type_estimate = Tok::Identifier;
                    };
                    break;
                }
                case '\n': {
                    ++f_loc.line;
                    break;
                }
                case '{' : {
                    *bufferEnd = c;
                    uint64_t len = bufferEnd - bufferStart;
                    f_loc.startCol = f_loc.endCol - len;
                    pushTokenToVector(token_vector,bufferEnd,len,Tok::OpenBrace,f_loc);
                    RESET;
                    break;
                }
                case '}' : {
                    *bufferEnd = c;
                    uint64_t len = bufferEnd - bufferStart;
                    f_loc.startCol = f_loc.endCol - len;
                    pushTokenToVector(token_vector,bufferEnd,len,Tok::CloseBrace,f_loc);
                    RESET;
                    break;
                }
                default : {
                    if(isalnum(c)){
                        *bufferEnd = c;
                        ++bufferEnd;
                        c = aheadChar();
                        if(isspace(c)){
                            uint64_t len = bufferEnd - bufferStart;
                            f_loc.startCol = f_loc.endCol - len;
                            pushTokenToVector(token_vector,bufferEnd,len,type_estimate,f_loc);
                            RESET;
                        };
                    };
                    break;
                };
            }
            ++f_loc.endCol;
            c = nextChar();
        }
    };

     Keyword matchKeyword(std::string & str){
        if(str == ATVERTEX_KW){
            return Keyword::AtVertex;
        }
        else if(str == ATFRAGMENT_KW){
            return Keyword::AtFragment;
        }
        else if(str == ATCOMPUTE_KW){
            return Keyword::AtCompute;
        }
        else if(str == STRUCT_KW){
            return Keyword::Struct;
        }
        return Keyword::UnknownOrOther;
    };
};