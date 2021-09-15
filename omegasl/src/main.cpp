#include <omega-common/common.h>

#include "Parser.h"
#include "CodeGen.h"

#include <iostream>

inline void help(){
    std::cout <<
    R"(Usage: omegaslc [options] [required] input-file

Required:
    --temp-dir, -t       --> Set the temp file output dir (For byproducts of compiling the lib)
    --output-dir,-o      --> Set the output dir of main compilation products (*.omegasllib and interface)
Options:

    --help ,    -h       --> Show this message.
    --tokens-only        --> Show tokens of all input files.
    --interface-only     --> Emit interface of all input files.


    --hlsl          --> Generate HLSL code.
    --metal         --> Generate Metal Shading Language code.
    --glsl          --> Generate GLSL code.
    )" << std::endl;
}

enum class GenMode : int {
    glsl,
    hlsl,
    metal,
    unknown
};

GenMode defaultGenModeForHost(){
    #ifdef _WIN32
        return GenMode::hlsl;
    #elif defined(__APPLE__)
        return GenMode::metal;
    #else
        return GenMode::glsl;
    #endif
}

int main(int argc,char *argv[]){

    bool tokenize = false;
    bool interfaceOnly = false;

    GenMode genMode = GenMode::unknown;
    const char * outputDir = nullptr,*tempDir = nullptr;
    OmegaCommon::StrRef inputFile = argv[argc - 1];

    for(unsigned i = 1;i < argc;i++){
        OmegaCommon::StrRef arg{argv[i]};
        if(arg == "--help" || arg == "-h"){
            help();
            return 0;
        }
        else if(arg == "--metal"){
#ifdef TARGET_METAL
            genMode = GenMode::metal;
#else
            std::cerr << "Metal code can only be compiled on an Apple Device." << std::endl;
            return 1;
#endif
        }
        else if(arg == "--hlsl"){
#ifdef TARGET_DIRECTX
            genMode = GenMode::hlsl;
#else
            std::cerr << "HLSL code can only be compiled on a Windows Device." << std::endl;
            return 1;
#endif
        }
        else if(arg == "--glsl"){
#ifdef TARGET_VULKAN
            genMode = GenMode::glsl;
#else
            std::cerr << "GLSL code can only be compiled on a Linux Device." << std::endl;
            return 1;
#endif
        }
        else if(arg == "--tokens-only"){
            tokenize = true;
        }
        else if(arg == "--interface-only"){
            interfaceOnly = true;
        }
        else if(arg == "--output-dir" || arg == "-o"){
            outputDir = argv[++i];
        }
        else if(arg == "--temp-dir" || arg == "-t"){
            tempDir = argv[++i];
        }
    }


    if(tempDir == nullptr){
        std::cout << "Temp Directory is not set" << std::endl;
    }

    if(outputDir == nullptr){
        std::cout << "Output Directory is not set" << std::endl;
        exit(1);
    }

    if(tempDir == nullptr){
        exit(1);
    }

    OmegaCommon::FS::Path outputPath(outputDir);

    if(!OmegaCommon::FS::exists(outputPath)){
        OmegaCommon::FS::createDirectory(outputPath);
    };

    OmegaCommon::FS::Path tempPath(tempDir);

    if(!OmegaCommon::FS::exists(tempPath)){
        OmegaCommon::FS::createDirectory(tempPath);
    };

    if(!OmegaCommon::FS::exists(inputFile)){
        std::cout << "File `" << inputFile << "` does not exist." << std::endl;
        return 1;
    }

    std::ifstream in(inputFile.data(),std::ios::in);

    if(tokenize){
        omegasl::Lexer lexer;
        lexer.setInputStream(&in);
        omegasl::Tok t;
        while((t = lexer.nextTok()).type != TOK_EOF){
            std::cout << "Tok {type:" << std::hex << t.type << std::dec << ", str:`" << t.str << "`}" << std::endl;
        }
        lexer.finishTokenizeFromStream();
        in.close();
        return 0;
    }

    if(genMode == GenMode::unknown){
        genMode = defaultGenModeForHost();
    }

    std::shared_ptr<omegasl::CodeGen> codeGen;

    omegasl::CodeGenOpts codeGenOpts {interfaceOnly,false,outputDir,tempDir};
    omegasl::MetalCodeOpts metalCodeOpts {};
    omegasl::GLSLCodeOpts glslCodeOpts {};

    if(genMode == GenMode::hlsl){
        codeGen = omegasl::HLSLCodeGenMake(codeGenOpts);
    }
    else if(genMode == GenMode::metal){
        codeGen = omegasl::MetalCodeGenMake(codeGenOpts,metalCodeOpts);
    }
    else {
        codeGen = omegasl::GLSLCodeGenMake(codeGenOpts,glslCodeOpts);
    }


    omegasl::Parser parser(codeGen);
    parser.parseContext({in});

    return 0;
};
