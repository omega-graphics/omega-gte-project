#include "omegaGTE/GE.h"
#include "omegaGTE/GTEShader.h"

#ifdef TARGET_DIRECTX
#include "d3d12/GED3D12.h"
#endif



#ifdef TARGET_METAL
#include "metal/GEMetal.h"
#endif



#ifdef TARGET_VULKAN
#include "vulkan/GEVulkan.h"
#endif

_NAMESPACE_BEGIN_

typedef unsigned char ShaderByte;

SharedHandle<GTEShaderLibrary> OmegaGraphicsEngine::loadShaderLibraryFromInputStream(std::istream &in) {
    auto lib = std::make_shared<GTEShaderLibrary>();

    std::string libName;
    StrRef::size_type size;
    in.read((char *)&size,sizeof(size));
    libName.resize(size);
    in.read(libName.data(),libName.size());
    unsigned entryCount;
    in.read((char *)&entryCount,sizeof(entryCount));
    for(;entryCount > 0;entryCount--){
        /// 1. Read Shader Type.
        omegasl_shader shaderEntry {};
        in.read((char *)&shaderEntry.type,sizeof(shaderEntry.type));
        /// 2. Read Shader Name Length and Data
        size_t name_len;
        in.read((char *)&name_len,sizeof(name_len));
        shaderEntry.name = new char[name_len];
        in.read((char *)shaderEntry.name,name_len);

        /// 3. Read Shader GPU Code Length and Data
        in.read((char *)&shaderEntry.dataSize,sizeof(shaderEntry.dataSize));
        shaderEntry.data = new ShaderByte [shaderEntry.dataSize];
        in.read((char *)shaderEntry.data,shaderEntry.dataSize);

        /// 4. Read Shader Layout
        in.read((char *)&shaderEntry.nLayout,sizeof(shaderEntry.nLayout));

        auto layout_count = shaderEntry.nLayout;

        auto layoutDescArr = new omegasl_shader_layout_desc[layout_count];

        for(unsigned i = 0;i < layout_count;i++){
            in.read((char *)(layoutDescArr + i),sizeof(omegasl_shader_layout_desc));
        }

        shaderEntry.pLayout = layoutDescArr;

        lib->shaders.insert(std::make_pair(OmegaCommon::StrRef((char *)shaderEntry.name,(unsigned)name_len), _loadShaderFromDesc(&shaderEntry)));
    }
    return lib;
}

SharedHandle<GTEShaderLibrary> OmegaGraphicsEngine::loadShaderLibrary(FS::Path path) {
    assert(path.exists() && "Path does not exist!");
    std::ifstream in(path.absPath(),std::ios::in | std::ios::binary);
    auto res = loadShaderLibraryFromInputStream(in);
    in.close();
    return res;
}

SharedHandle<OmegaGraphicsEngine> OmegaGraphicsEngine::Create(){
    #ifdef TARGET_METAL
        return CreateMetalEngine();
    #endif
    #ifdef TARGET_DIRECTX
        return GED3D12Engine::Create();
    #endif
    #ifdef TARGET_VULKAN
        return GEVulkanEngine::Create();
    #endif
};

_NAMESPACE_END_
