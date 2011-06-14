MACRO(_MACRO_CREATE_QT_COMPRESSED_HELP qch_file)
   
  CONFIGURE_FILE(${BlueBerry_SOURCE_DIR}/CMake/berryQtHelpDoxygen.conf.in
                 ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                 )

  SET(_generated_qch_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/qch/${BUNDLE-NAME}.qch")
  SET(${qch_file} "${PLUGIN_OUTPUT_DIR}/resources/${BUNDLE-SYMBOLICNAME}.qch")

  FILE(GLOB _file_dependencies "${PLUGIN_DOXYGEN_INPUT_DIR}/*.dox")
  
  ADD_CUSTOM_COMMAND(OUTPUT ${${qch_file}}
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_generated_qch_file} ${${qch_file}} 
                     DEPENDS ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf ${_file_dependencies}
                     )
                     
  #SET_SOURCE_FILES_PROPERTIES(${qch_file} PROPERTIES GENERATED 1)
       
ENDMACRO()

FUNCTION(_FUNCTION_CREATE_CTK_QT_COMPRESSED_HELP qch_file)

  SET(_manifest_path "${CMAKE_CURRENT_SOURCE_DIR}/manifest_headers.cmake")
  IF(NOT EXISTS ${_manifest_path})
    MESSAGE(FATAL_ERROR "${_manifest_path} not found")
  ENDIF()

  INCLUDE(${_manifest_path})
  STRING(REPLACE "_" "." Plugin-SymbolicName "${PLUGIN_TARGET}")

  CONFIGURE_FILE(${BlueBerry_SOURCE_DIR}/CMake/berryCTKQtHelpDoxygen.conf.in
                 ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                 )

  SET(_generated_qch_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/qch/${Plugin-Name}.qch")
  SET(${qch_file} "${CMAKE_CURRENT_BINARY_DIR}/resources/${Plugin-SymbolicName}.qch")

  FILE(GLOB _file_dependencies "${PLUGIN_DOXYGEN_INPUT_DIR}/*.dox")

  ADD_CUSTOM_COMMAND(OUTPUT ${${qch_file}}
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_generated_qch_file} ${${qch_file}}
                     DEPENDS ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf ${_file_dependencies}
                     )

  #SET_SOURCE_FILES_PROPERTIES(${qch_file} PROPERTIES GENERATED 1)

  SET(${qch_file} ${${qch_file}} PARENT_SCOPE)

ENDFUNCTION()
