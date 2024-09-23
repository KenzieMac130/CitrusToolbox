function(add_citrus_codegen _CODEGEN_TARGET_NAME _CODEGEN_INPUTS)
# create input list file
file(WRITE ${CITRUS_CODEGEN_DIR}/${_CODEGEN_TARGET_NAME}.txt "${_CODEGEN_INPUTS}")

# create outputs for each file 
execute_process(
   COMMAND python ${CMAKE_SOURCE_DIR}/tools/codegen/InitializeOutputs.py ${CITRUS_CODEGEN_DIR} ${CMAKE_SOURCE_DIR} ${CITRUS_CODEGEN_DIR}/${_CODEGEN_TARGET_NAME}.txt
   OUTPUT_VARIABLE CT_CODEGEN_OUTPUTS
   RESULT_VARIABLE RETURN_VALUE
)
if (NOT RETURN_VALUE EQUAL 0)
    message(FATAL_ERROR "Failed to initialize output")
endif()

# create codegen generation target
add_custom_command(
    COMMAND CitrusCodegen ${CITRUS_CODEGEN_DIR} ${CMAKE_SOURCE_DIR} ${CITRUS_CODEGEN_DIR}/${_CODEGEN_TARGET_NAME}.txt
    DEPENDS CitrusCodegen ${_CODEGEN_INPUTS}
    OUTPUT ${CT_CODEGEN_OUTPUTS}
)

# create codegen output library
add_library(${_CODEGEN_TARGET_NAME} ${CT_CODEGEN_OUTPUTS})
set_target_properties(${_CODEGEN_TARGET_NAME} PROPERTIES FOLDER "codegen")
endfunction()