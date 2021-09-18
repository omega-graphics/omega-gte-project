#include <omega-common/common.h>

#include "Parser.h"
#include "CodeGen.h"

#include <iostream>

inline void help(){
    std::cout <<
    R"(Usage: omegaslc [options] [required] input-file

Required:
    --temp-dir, -t                  --> Set the temp file output dir (For byproducts of compiling the lib)
    --output,-o                     --> Set the output *.omegasllib.
Options:

    --help ,    -h                  --> Show this message.
    --tokens-only                   --> Show tokens of all input files.
    --interface-only                --> Emit interface of all input files.


    --hlsl                          --> Generate HLSL code.
    --metal                         --> Generate Metal Shading Language code.
    --glsl                          --> Generate GLSL code.

Metal Options:
    --target-arch=[x86_64,aarch64]  --> Select the target architecture to compile the MSL to.
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
    const char * outputLibn = nullptr,*tempDir = nullptr;
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
        else if(arg == "--output" || arg == "-o"){
            outputLibn = argv[++i];
        }
        else if(arg == "--temp-dir" || arg == "-t"){
            tempDir = argv[++i];
        }
    }


    if(tempDir == nullptr){
        std::cout << "Temp Directory is not set" << std::endl;
    }

    if(outputLibn == nullptr){
        std::cout << "Output Lib is not set" << std::endl;
        exit(1);
    }

    if(tempDir == nullptr){
        exit(1);
    }

    OmegaCommon::FS::Path outputLib(outputLibn);
    auto outputPath = OmegaCommon::FS::Path(OmegaCommon::FS::Path(outputLib).dir());

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

    auto input_file_path = OmegaCommon::FS::Path(inputFile);

    std::ifstream in(inputFile.data(),std::ios::in);

    if(tokenize){
        auto lexer = OmegaCommon::makeARCAny<omegasl::Lexer>();
        lexer->setInputStream(&in);
        omegasl::Tok t;
        while((t = lexer->nextTok()).type != TOK_EOF){
            std::cout << "Tok {type:" << std::hex << t.type << std::dec << ", str:`" << t.str << "`}" << std::endl;
        }
        lexer->finishTokenizeFromStream();
        in.close();
        return 0;
    }

    if(genMode == GenMode::unknown){
        genMode = defaultGenModeForHost();
    }

    std::shared_ptr<omegasl::CodeGen> codeGen;

    omegasl::CodeGenOpts codeGenOpts {interfaceOnly,false,outputLibn,tempDir};
    omegasl::MetalCodeOpts metalCodeOpts {};
    omegasl::GLSLCodeOpts glslCodeOpts {};
    omegasl::HLSLCodeOpts hlslCodeOpts {};

    if(genMode == GenMode::hlsl){
#ifdef TARGET_DIRECTX
        hlslCodeOpts.dxc_cmd = "dxc";
#endif
        codeGen = omegasl::HLSLCodeGenMake(codeGenOpts);
    }
    else if(genMode == GenMode::metal){
#ifdef TARGET_METAL
        metalCodeOpts.mtl_device = nullptr;
        metalCodeOpts.metal_cmd = "xcrun -sdk macosx metal";
#endif
        codeGen = omegasl::MetalCodeGenMake(codeGenOpts,metalCodeOpts);
    }
    else {
        codeGen = omegasl::GLSLCodeGenMake(codeGenOpts,glslCodeOpts);
    }


    omegasl::Parser parser(codeGen);
    parser.parseContext({in});

    codeGen->linkShaderObjects();

    return 0;
};
