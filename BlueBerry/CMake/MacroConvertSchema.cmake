# MACRO_CONVERT_SCHEMA(INPUT plugin.xml ...
#                      OUTPUT_DIR output_dir
#                      TARGET_NAME target_name
#                      [DEPENDS depends]
#                      [WORKING_DIR working_dir]
#                     )

macro(MACRO_CONVERT_SCHEMA)

MACRO_PARSE_ARGUMENTS(_SCHEMA "INPUT;OUTPUT_DIR;TARGET_NAME;DEPENDS;WORKING_DIR" "" ${ARGN})

set(XP_ANT_TASKS)
foreach(_file ${_SCHEMA_INPUT})
  set(XP_ANT_TASKS "${XP_ANT_TASKS}
  <pde.convertSchemaToHTML manifest=\"${_file}\" destination=\"\${dest}\" />")
endforeach(_file ${_SCHEMA_INPUT})

set(XP_DOC_DESTINATION "${_SCHEMA_OUTPUT_DIR}")

if(NOT _SCHEMA_WORKING_DIR)
  set(_SCHEMA_WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif(NOT _SCHEMA_WORKING_DIR)

configure_file("${BLUEBERRY_SOURCE_DIR}/Build/ConvertSchemaToHTML/buildExtPointDoc.xml.in"
  "${_SCHEMA_WORKING_DIR}/buildExtPointDoc.xml" @ONLY)

add_custom_target(${_SCHEMA_TARGET_NAME}
                  ant -f ${_SCHEMA_WORKING_DIR}/buildExtPointDoc.xml
                  DEPENDS ${_SCHEMA_DEPENDS}
                  WORKING_DIRECTORY ${_SCHEMA_WORKING_DIR}
                  VERBATIM)

endmacro(MACRO_CONVERT_SCHEMA)