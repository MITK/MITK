# MACRO_CONVERT_SCHEMA(INPUT plugin.xml ...
#                      OUTPUT_DIR output_dir
#                      TARGET_NAME target_name
#                      [DEPENDS depends]
#                      [WORKING_DIR working_dir]
#                     )

MACRO(MACRO_CONVERT_SCHEMA)

MACRO_PARSE_ARGUMENTS(_SCHEMA "INPUT;OUTPUT_DIR;TARGET_NAME;DEPENDS;WORKING_DIR" "" ${ARGN})

SET(XP_ANT_TASKS)
FOREACH(_file ${_SCHEMA_INPUT})
  SET(XP_ANT_TASKS "${XP_ANT_TASKS}
  <pde.convertSchemaToHTML manifest=\"${_file}\" destination=\"\${dest}\" />")
ENDFOREACH(_file ${_SCHEMA_INPUT})

SET(XP_DOC_DESTINATION "${_SCHEMA_OUTPUT_DIR}")

IF(NOT _SCHEMA_WORKING_DIR)
  SET(_SCHEMA_WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})
ENDIF(NOT _SCHEMA_WORKING_DIR)

CONFIGURE_FILE("${OPENCHERRY_SOURCE_DIR}/Build/ConvertSchemaToHTML/buildExtPointDoc.xml.in" 
  "${_SCHEMA_WORKING_DIR}/buildExtPointDoc.xml" @ONLY)
  
ADD_CUSTOM_TARGET(${_SCHEMA_TARGET_NAME} 
                  ant -f ${_SCHEMA_WORKING_DIR}/buildExtPointDoc.xml
                  DEPENDS ${_SCHEMA_DEPENDS}
                  WORKING_DIRECTORY ${_SCHEMA_WORKING_DIR}
                  VERBATIM)

ENDMACRO(MACRO_CONVERT_SCHEMA)