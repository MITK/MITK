# mitkFunctionConvertXPSchema(INPUT plugin.xml ...
#                      OUTPUT_DIR output_dir
#                      TARGET_NAME target_name
#                      [DEPENDS depends]
#                      [WORKING_DIR working_dir]
#                     )
function(mitkFunctionConvertXPSchema)

  set(_macro_options
  )

  set(_macro_params
    OUTPUT_DIR
    TARGET_NAME
    WORKING_DIR
  )

  set(_macro_multiparams
   INPUT
   DEPENDS
  )

  cmake_parse_arguments(_SCHEMA "${_macro_options}" "${_macro_params}" "${_macro_multiparams}" ${ARGN})

  set(XP_ANT_TASKS)
  foreach(_file ${_SCHEMA_INPUT})
    set(XP_ANT_TASKS "${XP_ANT_TASKS}
      <pde.convertSchemaToHTML manifest=\"${_file}\" destination=\"\${dest}\" />"
      )
  endforeach()

  set(XP_DOC_DESTINATION "${_SCHEMA_OUTPUT_DIR}")

  if(NOT _SCHEMA_WORKING_DIR)
    set(_SCHEMA_WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  configure_file("${CMAKE_CURRENT_LIST_DIR}/buildExtPointDoc.xml.in"
                 "${_SCHEMA_WORKING_DIR}/buildExtPointDoc.xml" @ONLY)

  add_custom_target(${_SCHEMA_TARGET_NAME}
                    ant -f ${_SCHEMA_WORKING_DIR}/buildExtPointDoc.xml
                    DEPENDS ${_SCHEMA_DEPENDS}
                    WORKING_DIRECTORY ${_SCHEMA_WORKING_DIR}
                    VERBATIM)

endfunction()
