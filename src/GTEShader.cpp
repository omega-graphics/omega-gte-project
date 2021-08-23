#include "omegaGTE/GTEShader.h"
#include "GEPipeline.cpp"

namespace OmegaGTE {

    GTEShaderBuilder::GTEShaderBuilder(OmegaGraphicsEngine *engine):engine(engine){

    }

    GTEShaderBuilder & GTEShaderBuilder::startShader(Shader::Type type) {
        #ifdef TARGET_DIRECTX

        #elif defined(TARGET_METAL)

        #endif
        return *this;
    }

    SharedHandle<GTEShader> GTEShaderBuilder::finishShader(){
        auto strSource = out.str();
        out.str("");
        return nullptr;
    };
}