include(CMakeParseArguments)

function(add_d3d12_test)
    cmake_parse_arguments("_ARG" "" "NAME" "SOURCES;LIBS" ${ARGN})
    
    add_executable(${_ARG_NAME} WIN32 ${_ARG_SOURCES})
    target_link_libraries(${_ARG_NAME} PRIVATE "OmegaGTE" ${_ARG_LIBS})
    add_custom_command(TARGET ${_ARG_NAME} POST_BUILD 
        COMMAND py -3 ${CMAKE_SOURCE_DIR}/gn/__copy.py --src ${CMAKE_BINARY_DIR}/OmegaGTE.dll --dest ${CMAKE_CURRENT_BINARY_DIR}/OmegaGTE.dll)
    # target_include_directories(${_ARG_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
endfunction()


add_d3d12_test(
    NAME 
    "2DTest"
    SOURCES
    "2DTest/main.cpp"
)
