include(CMakeParseArguments)

function(add_metal_test)
    cmake_parse_arguments("_ARG" "" "NAME;PLIST" "SOURCES;RESOURCES;FRAMEWORKS;LIBS" ${ARGN})
    add_executable(${_ARG_NAME} MACOSX_BUNDLE ${_ARG_SOURCES} ${_ARG_RESOURCES})
    
    set_target_properties(${_ARG_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${_ARG_PLIST} RESOURCE "${_ARG_RESOURCES}")

    set(_FRAMEWORKS)
    foreach(f ${_ARG_FRAMEWORKS})
        find_library("__${f}" ${f})
        list(APPEND _FRAMEWORKS "${__${f}}")
    endforeach()
    
    target_link_libraries(${_ARG_NAME} PRIVATE "OmegaGTE" ${_ARG_LIBS} ${_FRAMEWORKS})
endfunction()


add_metal_test(
    NAME 
    "2DTest"
    SOURCES
    "2DTest/main.mm"
    RESOURCES
    "2DTest/English.lproj/MainMenu.xib"
    PLIST 
    "${CMAKE_CURRENT_SOURCE_DIR}/2DTest/Info.plist"
    FRAMEWORKS 
    "Cocoa"
)