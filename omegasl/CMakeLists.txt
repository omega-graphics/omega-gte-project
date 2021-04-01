message("OmegaSL Will Build")

find_package(LLVM REQUIRED CONFIG)


message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

llvm_map_components_to_libnames(llvm_libs support core irreader)

add_executable("omegasl" "main.cpp" "OmegaSLGen.cpp" "OmegaSLGen.h" "OmegaSLSyntax.h"  "OmegaSLSyntax.cpp")
target_compile_definitions("omegasl" PRIVATE ${LLVM_DEFINITIONS})
target_include_directories("omegasl" PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries("omegasl" PRIVATE ${llvm_libs})