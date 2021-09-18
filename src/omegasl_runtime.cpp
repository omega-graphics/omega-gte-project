#include "omegasl.h"
#include "../omegasl/src/CodeGen.h"
#include "../omegasl/src/Parser.h"

#ifdef RUNTIME_SHADER_COMP_SUPPORT

struct SourceImpl : public OmegaSLCompiler::Source {
    bool file;
    std::ifstream in_file;
    std::istringstream in;
    explicit SourceImpl(OmegaCommon::FS::Path & path):file(true),in_file(path.absPath(),std::ios::in){

    };
    explicit SourceImpl(OmegaCommon::String & buffer):file(false),in(buffer){

    }
};

std::shared_ptr<OmegaSLCompiler::Source> OmegaSLCompiler::Source::fromFile(OmegaCommon::FS::Path path) {
    return std::shared_ptr<SourceImpl>(new SourceImpl(path));
}

std::shared_ptr<OmegaSLCompiler::Source> OmegaSLCompiler::Source::fromString(OmegaCommon::String &buffer) {
    return std::shared_ptr<SourceImpl>(new SourceImpl(buffer));
}


class OmegaSLCompilerImpl : public OmegaSLCompiler {
    std::shared_ptr<omegasl::CodeGen> gen;
    std::shared_ptr<omegasl::Parser> parser;
public:
    OmegaSLCompilerImpl(){
        omegasl::CodeGenOpts genOpts {false,true};
#if defined(TARGET_DIRECTX)
        gen = omegasl::HLSLCodeGenMake(genOpts);
#elif defined(TARGET_METAL)
        omegasl::MetalCodeOpts metalCodeOpts {"",""};
        gen = omegasl::MetalCodeGenMake(genOpts,metalCodeOpts);
#endif
        parser = std::make_shared<omegasl::Parser>(gen);

    }
    std::shared_ptr<omegasl_shader_lib> compile(std::initializer_list<std::shared_ptr<Source>> sources) override {
       for(auto & s : sources){
           auto source = (SourceImpl *)s.get();
           std::istream *in = nullptr;
           if(source->file){
                in = &source->in_file;
           }
           else {
              in = &source->in;
           }
           omegasl::ParseContext context {*in};
           parser->parseContext(context);
       }
       auto res = gen->getLibrary("RUNTIME");
       gen->resetShaderMap();
       return res;
    }
};

std::shared_ptr<OmegaSLCompiler> OmegaSLCompiler::Create() {
    return std::make_shared<OmegaSLCompilerImpl>();
}



#endif