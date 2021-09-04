cmake_minimum_required(VERSION 3.9)

include(OmegaGraphicsSuite)

set(OMEGAGTE_VERSION "0.5")

omega_graphics_project(OmegaGTE VERSION ${OMEGAGTE_VERSION} LANGUAGES CXX)

omega_graphics_add_subdir(OmegaCommon common)


include(./OmegaGTE.cmake)

file(GLOB MAIN_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(WIN32)

	set(TARGET_DIRECTX TRUE)

	add_compile_definitions("TARGET_DIRECTX")

	file(GLOB RHI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/d3d12/*.cpp")

elseif(APPLE)

	set(TARGET_METAL TRUE)

	add_compile_definitions("TARGET_METAL")

	file(GLOB RHI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/metal/*.mm")

else()
	set(TARGET_VULKAN TRUE)

	add_compile_definitions("TARGET_VULKAN")

	file(GLOB RHI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/*.cpp")
endif()

set(MAIN_SRCS ${MAIN_SRCS} ${RHI_SRCS})

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)

file(GLOB OMEGASL_SRCS "omegasl/src/*.h" "omegasl/src/*.cpp")

add_omega_graphics_tool("omegaslc" SOURCES ${OMEGASL_SRCS} LIBS "OmegaCommon")
target_include_directories("omegaslc" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_omega_graphics_module("OmegaGTE" FRAMEWORK INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" VERSION "0.5" HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" DEPENDS "OmegaCommon" SOURCES ${MAIN_SRCS})
target_compile_definitions("OmegaGTE" PRIVATE OMEGAGTE__BUILD__) 
target_link_libraries("OmegaGTE" PUBLIC "OmegaCommon")
if(TARGET_METAL)
	target_link_system_frameworks("OmegaGTE" Metal QuartzCore Cocoa)
endif()
# add_subdirectory(tests)

