#include "Parser.h"
#include "Gen.h"
#include <initializer_list>
#include <string>
#include <optional>
#include <string_view>
#include <vector>
#include <iostream>
namespace omegasl::cmdline {

    struct Input {
        std::string_view flag;
        std::optional<std::string_view> flag2;
        std::string * val;
    };

    struct Option {
        std::string_view flag;
        std::optional<std::string_view> flag2;
        bool *state;
    };

    void parse(std::initializer_list<Option *> opts,std::initializer_list<Input *> inputs,int & argc,char * argv[],std::vector<std::string> *unparsedArgs,const char * help){

        auto match = [&](std::string_view & arg,std::optional<std::string_view> &next_arg,bool * increment) -> void{
            *increment = false;
            if(arg == "-h" || arg == "--help"){
                std::cout << help << std::endl;
                exit(0);
            };

            for(auto opt : opts){
                if(opt->flag == arg){
                    *(opt->state) = true;
                    return;
                }
                else if(opt->flag2.has_value()){
                    if(opt->flag2 == arg){
                        *(opt->state) = true;
                        return;
                    };
                }
                else {
                    *(opt->state) = false;
                };
            };

            for(auto input : inputs){
                if(input->flag == arg){
                    *(input->val) = "";
                    return;
                }
                else if(input->flag2.has_value()){
                    if(input->flag2 == arg){
                        if(!next_arg.has_value()) {
                            std::cout << "ERROR: Expected Input Argument Exiting.." << std::endl;
                            exit(1);
                        }
                        input->val->resize(next_arg->size());
                        std::copy(next_arg->begin(),next_arg->end(),input->val->begin());
                        *increment = true;
                        return;
                    };
                }
                else {
                    *(input->val) = "";
                };
            };
            unparsedArgs->push_back(arg.data());
        };

        for(unsigned i = 1; i < argc;i++){
            bool increment = false;
            std::string_view strRef(argv[i]);
            std::optional<std::string_view> aheadStrRef;
            if((i + 1) < argc) aheadStrRef = std::string_view(argv[i + 1]);
            // std::cout << strRef << std::endl;
            match(strRef,aheadStrRef,&increment);
            if(increment)
                ++i;
        };
    };

};


int main(int argc,char *argv[]){
    
    std::vector<std::string> sources;
    omegasl::cmdline::parse({},{},argc,argv,&sources,
    "Omega Shading Language Compiler:\n"
    "\n"
    "Flags:\n"
    "--metal\n"
    "--glsl\n"
    "--hlsl\n");

    // for(auto & s :sources){
    //     std::cout << s << std::endl;
    // };

    return 0;
};