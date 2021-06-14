cmake_minimum_required(VERSION 3.15)

project(OmegaGTE C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


if(CMAKE_GENERATOR STREQUAL "Ninja")
    message(FATAL_ERROR "Ninja is not allowed wit the CMake Build System.. If you wish to build this project using Ninja, use our GN")
endif()


if(XCODE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_IDENTITY ${CODE_SIGNATURE})
    # set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${CODE_SIGNATURE})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

include(OmegaGTE)


set(_TARGET_SOURCES)

if(WIN32)
    set(TARGET_DIRECTX TRUE)
    add_compile_definitions("TARGET_DIRECTX")

    set(_TARGET_SOURCES
    "src/d3d12/GED3D12.h"
    "src/d3d12/GED3D12.cpp"
    "src/d3d12/GED3D12CommandQueue.h"
    "src/d3d12/GED3D12CommandQueue.cpp"
    "src/d3d12/GED3D12Pipeline.h"
    "src/d3d12/GED3D12Pipeline.cpp"
    "src/d3d12/GED3D12RenderTarget.h"
    "src/d3d12/GED3D12RenderTarget.cpp"
    "src/d3d12/GED3D12Texture.h"
    "src/d3d12/GED3D12Texture.cpp"
    "src/D3D12TEContext.cpp"
    )
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
    "src/metal/GEMetalPipeline.h"
    "src/metal/GEMetalPipeline.mm"
    "src/metal/GEMetalRenderTarget.h"
    "src/metal/GEMetalRenderTarget.mm"
    "src/metal/GEMetalTexture.h"
    "src/metal/GEMetalTexture.mm"
    "src/MetalTEContext.mm"
    )
endif()

if(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
    set(TARGET_VULKAN TRUE)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GTK REQUIRED "gtk+-3.0")
    if(GTK_FOUND)

    endif()
    find_package(Vulkan REQUIRED)
    if(VULKAN_FOUND)

    endif()
endif()

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
if(TARGET_METAL)
    find_library(METAL_FOUND "Metal")
    find_library(COREANIMATION_FOUND "QuartzCore")
    find_library(FOUNDATION_FOUND "Foundation")
    find_library(APPKIT_FOUND "AppKit")
    if(METAL_FOUND)
        message("Found Metal.framework (Path:${METAL_FOUND})")
    endif()
endif()

add_library("OmegaGTE" SHARED 
# Headers
"${INCLUDE_DIR}/OmegaGTE.h"
"${INCLUDE_DIR}/omegaGTE/GE.h"
"${INCLUDE_DIR}/omegaGTE/GECommandQueue.h" 
"${INCLUDE_DIR}/omegaGTE/GEPipeline.h" 
"${INCLUDE_DIR}/omegaGTE/GTEBase.h"
"${INCLUDE_DIR}/omegaGTE/GERenderTarget.h"
"${INCLUDE_DIR}/omegaGTE/GETexture.h"
"${INCLUDE_DIR}/omegaGTE/GTEShaderTypes.h"
"${INCLUDE_DIR}/omegaGTE/TE.h"      
# Sources
"src/GE.cpp"
"src/GECommandQueue.cpp"
"src/GEPipeline.cpp"
"src/GERenderTarget.cpp"
"src/GTEBase.cpp"
"src/TE.cpp"
"src/OmegaGTE.cpp"
${_TARGET_SOURCES})

add_omegasl_lib("std" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/std.omegasl" "${CMAKE_BINARY_DIR}/stdshaderlib")

# set_target_properties("OmegaGTE" PROPERTIES LINK_FLAGS "")

if(TARGET_METAL)

set_target_properties("OmegaGTE" PROPERTIES 
FRAMEWORK TRUE
FRAMEWORK_VERSION "0.2"
MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist"
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Frameworks")

target_sources("OmegaGTE" PRIVATE "${CMAKE_BINARY_DIR}/stdshaderlib")
set_source_files_properties("${CMAKE_BINARY_DIR}/stdshaderlib" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources" GENERATED TRUE)

target_link_libraries("OmegaGTE" PRIVATE ${METAL_FOUND} ${COREANIMATION_FOUND} ${FOUNDATION_FOUND} ${APPKIT_FOUND})
endif()

if(TARGET_VULKAN)
set_target_properties("OmegaGTE" PROPERTIES INCLUDE_DIRECTORIES ${GTK_INCLUDE_DIRS} LINK_DIRECTORIES ${GTK_LIBRARY_DIRS} COMPILE_FLAGS ${GTK_CFLAGS_OTHER} LINK_LIBRARIES ${GTK_LIBRARIES})
endif()

add_dependencies("OmegaGTE" "std")

target_include_directories("OmegaGTE" PUBLIC ${INCLUDE_DIR})


add_subdirectory("tests")
# add_subdirectory("omegasl")