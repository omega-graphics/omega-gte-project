cmake_minimum_required(VERSION 3.15)

project(OmegaGTE C CXX)

if(CMAKE_GENERATOR STREQUAL "Ninja")
    message(FATAL_ERROR "Ninja is not allowed wit the CMake Build System.. If you wish to build this project using Ninja, use our GN")
endif()

set(_TARGET_SOURCES)

if(WIN32)
    set(TARGET_DIRECTX TRUE)
    add_compile_definitions("TARGET_DIRECTX")
endif()

if(CMAKE_HOST_APPLE)
    set(TARGET_METAL TRUE)
    add_compile_definitions("TARGET_METAL")
    enable_language(OBJCXX)

    set(_TARGET_SOURCES 
    "src/metal/GEMetal.h"
    "src/metal/GEMetal.mm"
    "src/metal/GEMetalCommandQueue.h"
    "src/metal/GEMetalCommandQueue.mm"
    "src/metal/GEMetalTexture.h"
    "src/metal/GEMetalTexture.mm"
    )
endif()

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

find_library(METAL_FOUND "Metal")
if(METAL_FOUND)
    message("Found Metal.framework (Path:${METAL_FOUND})")
endif()

add_library("OmegaGTE" SHARED 
# Headers
"${INCLUDE_DIR}/omegaGTE/GE.h"
"${INCLUDE_DIR}/omegaGTE/GECommandQueue.h" 
"${INCLUDE_DIR}/omegaGTE/GEPipeline.h" 
"${INCLUDE_DIR}/omegaGTE/GTEBase.h"
"${INCLUDE_DIR}/omegaGTE/GERenderPass.h"
"${INCLUDE_DIR}/omegaGTE/GERenderTarget.h"
"${INCLUDE_DIR}/omegaGTE/GTEShaderTypes.h"
"${INCLUDE_DIR}/omegaGTE/TE.h"      
# Sources
"src/GE.cpp"
"src/GECommandQueue.cpp"
"src/GEPipeline.cpp"
"src/GERenderPass.cpp"
"src/GERenderTarget.cpp"
"src/GTEBase.cpp"
"src/TE.cpp"
${_TARGET_SOURCES})


if(TARGET_METAL)

set_target_properties("OmegaGTE" PROPERTIES 
FRAMEWORK TRUE
FRAMEWORK_VERSION "0.1"
FRAMEWORK_IDENTIFIER "org.omegagraphics.OmegaGTE")

endif()
target_include_directories("OmegaGTE" PUBLIC ${INCLUDE_DIR})
target_link_libraries("OmegaGTE" PRIVATE ${METAL_FOUND})