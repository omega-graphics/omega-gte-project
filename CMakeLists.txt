cmake_minimum_required(VERSION 3.9)

project(OmegaGTE CXX)

add_subdirectory(common)

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


add_omega_graphics_module("OmegaGTE" FRAMEWORK HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" DEPENDS "OmegaCommon" SOURCES ${MAIN_SRCS})
target_compile_definitions("OmegaGTE" PRIVATE "__BUILD__") 
target_link_libraries("OmegaGTE" PUBLIC "OmegaCommon")
add_subdirectory(tests)

