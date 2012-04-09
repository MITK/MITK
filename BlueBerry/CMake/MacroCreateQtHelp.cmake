
function(_FUNCTION_CREATE_CTK_QT_COMPRESSED_HELP qch_file use_xsl)

  set(_manifest_path "${CMAKE_CURRENT_SOURCE_DIR}/manifest_headers.cmake")
  if(NOT EXISTS ${_manifest_path})
    message(FATAL_ERROR "${_manifest_path} not found")
  endif()

  include(${_manifest_path})
  string(REPLACE "_" "." Plugin-SymbolicName "${PLUGIN_TARGET}")

  configure_file(${BlueBerry_SOURCE_DIR}/CMake/berryCTKQtHelpDoxygen.conf.in
                 ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                 )

  set(_qhp_xsl_file "${BlueBerry_SOURCE_DIR}/CMake/qhp_toc.xsl")
  set(_generated_qhp_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/html/index.qhp")
  set(_transformed_qhp_file "${PLUGIN_DOXYGEN_OUTPUT_DIR}/html/${PLUGIN_TARGET}.qhp")
  set(${qch_file} "${CMAKE_CURRENT_BINARY_DIR}/resources/${PLUGIN_TARGET}.qch")

  set(_xsl_command )
  if(use_xsl)
    set(_xsl_command COMMAND ${QT_XMLPATTERNS_EXECUTABLE} ${_qhp_xsl_file} ${_generated_qhp_file} -output ${_transformed_qhp_file})
  endif()

  file(GLOB _file_dependencies "${PLUGIN_DOXYGEN_INPUT_DIR}/*")

  add_custom_command(OUTPUT ${${qch_file}}
                     # Generate a Qt help project (index.qhp) with doxygen
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf
                     # Use a XSL transformation to get rid of the top-level entry
                     ${_xsl_command}
                     # Generate the final Qt compressed help file (.qch)
                     COMMAND ${QT_HELPGENERATOR_EXECUTABLE} ${_transformed_qhp_file} -o ${${qch_file}}
                     DEPENDS ${PLUGIN_DOXYGEN_OUTPUT_DIR}/doxygen.conf ${_file_dependencies}
                     )

  #set_source_files_properties(${qch_file} PROPERTIES GENERATED 1)

  set(${qch_file} ${${qch_file}} PARENT_SCOPE)

endfunction()
