MACRO(_MACRO_CREATE_QT_COMPRESSED_HELP qch_file)
  
  CONFIGURE_FILE(${openCherry_SOURCE_DIR}/CMake/cherryQtHelpDoxygen.conf.in
                 ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                 )

  SET(_generated_qch_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/qch/${BUNDLE-SYMBOLICNAME}.qch")
  SET(${qch_file} "${PLUGIN_OUTPUT_DIR}/resources/${BUNDLE-SYMBOLICNAME}.qch")

  FILE(GLOB _file_dependencies "${PLUGIN_DOXYGEN_INPUT_DIR}/*.dox")
  
  ADD_CUSTOM_COMMAND(OUTPUT ${${qch_file}}
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_generated_qch_file} ${${qch_file}} 
                     DEPENDS ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf ${_file_dependencies}
                     )
                     
  #SET_SOURCE_FILES_PROPERTIES(${qch_file} PROPERTIES GENERATED 1)
       
ENDMACRO()