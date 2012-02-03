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

FUNCTION(_FUNCTION_CREATE_CTK_QT_COMPRESSED_HELP qch_file use_xsl)

  SET(_manifest_path "${CMAKE_CURRENT_SOURCE_DIR}/manifest_headers.cmake")
  IF(NOT EXISTS ${_manifest_path})
    MESSAGE(FATAL_ERROR "${_manifest_path} not found")
  ENDIF()

  INCLUDE(${_manifest_path})
  STRING(REPLACE "_" "." Plugin-SymbolicName "${PLUGIN_TARGET}")

  CONFIGURE_FILE(${BlueBerry_SOURCE_DIR}/CMake/berryCTKQtHelpDoxygen.conf.in
                 ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                 )

  SET(_qhp_xsl_file "${BlueBerry_SOURCE_DIR}/CMake/qhp_toc.xsl")
  SET(_generated_qhp_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/html/index.qhp")
  SET(_transformed_qhp_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/html/${PLUGIN_TARGET}.qhp")
  SET(${qch_file} "${CMAKE_CURRENT_BINARY_DIR}/resources/${PLUGIN_TARGET}.qch")

  SET(_xsl_command )
  IF(use_xsl)
    SET(_xsl_command COMMAND ${QT_XMLPATTERNS_EXECUTABLE} ${_qhp_xsl_file} ${_generated_qhp_file} -output ${_transformed_qhp_file})
  ENDIF()

  FILE(GLOB _file_dependencies "${PLUGIN_DOXYGEN_INPUT_DIR}/*")

  ADD_CUSTOM_COMMAND(OUTPUT ${${qch_file}}
                     # Generate a Qt help project (index.qhp) with doxygen
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                     # Use a XSL transformation to get rid of the top-level entry
                     ${_xsl_command}
                     # Generate the final Qt compressed help file (.qch)
                     COMMAND ${QT_HELPGENERATOR_EXECUTABLE} ${_transformed_qhp_file} -o ${${qch_file}}
                     DEPENDS ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf ${_file_dependencies}
                     )

  #SET_SOURCE_FILES_PROPERTIES(${qch_file} PROPERTIES GENERATED 1)

  SET(${qch_file} ${${qch_file}} PARENT_SCOPE)

ENDFUNCTION()
