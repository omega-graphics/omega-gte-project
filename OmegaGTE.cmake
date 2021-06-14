
if(NOT PYTHON_EXEC)
    set(PYTHON_EXEC)
    if(WIN32)
        set(PYTHON_EXEC "py -3")
    else()
        set(PYTHON_EXEC "python3")
    endif()
endif()



set(OMEGASLC_PY "${CMAKE_CURRENT_LIST_DIR}/omegasl/_main.py")

function(add_omegasl_lib _NAME _SRC _OUTPUT)

    set(TARGET_SHADER_ARCH)
    if(TARGET_METAL)
        set(TARGET_SHADER_ARCH "metal")
    endif()

    add_custom_target(${_NAME} DEPENDS ${_OUTPUT}/${_NAME}.shadermap)
    
    add_custom_command(OUTPUT ${_OUTPUT}/${_NAME}.shadermap
                       COMMAND ${PYTHON_EXEC} ${OMEGASLC_PY} --target ${TARGET_SHADER_ARCH} --temp ${CMAKE_CURRENT_BINARY_DIR} --out ${_OUTPUT} ${_SRC}
                       DEPENDS ${_SRC})
    
endfunction()
