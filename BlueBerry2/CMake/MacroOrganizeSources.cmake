# this macro gets a filelist as input and looks
# for corresponding h-files to add them to the project.
# 
# additionally files are grouped in source-groups.
#  
# No parameters explicitly declared here, because
# we want to allow for variable argument lists, which
# are later access by the keyword FOREACH(_file ${ARGV})
#  
# output: after calling the macro, files that were found
# correspondigly to the given files are stored in the
# variable:
# ${CORRESPONDING__H_FILES}
#
# GLOBBED__H_FILES
  

MACRO(MACRO_ORGANIZE_SOURCES)

  MACRO_PARSE_ARGUMENTS(_ORG "HEADER;SOURCE;TXX;DOC;MOC;GEN_QRC;GEN_UI;META;UI;QRC" "" ${ARGN})

  SET(CORRESPONDING__H_FILES "" )
  SET(GLOBBED__H_FILES "" )
  
  IF(_ORG_HEADER)
    FOREACH(_file ${_ORG_SOURCE})
      STRING(REGEX REPLACE "(.*)\\.(txx|cpp|c|cxx)$" "\\1.h" H_FILE ${_file})
      IF(EXISTS ${H_FILE})
        LIST(APPEND CORRESPONDING__H_FILES "${H_FILE}")
      ENDIF()
    ENDFOREACH()
  ELSE()
    FILE(GLOB_RECURSE GLOBBED__H_FILES *.h)
  ENDIF()
  
  IF(_ORG_GEN_QRC OR _ORG_GEN_UI OR _ORG_MOC)
    SOURCE_GROUP("Generated\\Qt QRC Source Files" FILES ${_ORG_GEN_QRC})
    SOURCE_GROUP("Generated\\Qt UI Header Files" FILES ${_ORG_GEN_UI})
    SOURCE_GROUP("Generated\\Qt MOC Source Files" FILES ${_ORG_MOC})
  ENDIF()
  
  #_MACRO_APPEND_TO_LIST(_ORG_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/")
  SOURCE_GROUP("== Source Files ==" FILES ${_ORG_SOURCE})
  
  #_MACRO_APPEND_TO_LIST(_ORG_TXX "${CMAKE_CURRENT_SOURCE_DIR}/")
  SOURCE_GROUP("== Template Files ==" FILES ${_ORG_TXX})
  
  #_MACRO_APPEND_TO_LIST(_ORG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/")
  SOURCE_GROUP("== Header Files ==" FILES ${_ORG_HEADER} ${_ORG_HEADER} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES})
  
  
  #_MACRO_APPEND_TO_LIST(_ORG_UI "${CMAKE_CURRENT_SOURCE_DIR}/")
  SOURCE_GROUP("QT UI Files" FILES ${_ORG_UI})
  
  #_MACRO_APPEND_TO_LIST(_ORG_DOC "${CMAKE_CURRENT_SOURCE_DIR}/")
  SOURCE_GROUP("Doxygen Files" FILES ${_ORG_DOC})
  
  #_MACRO_APPEND_TO_LIST(_ORG_QRC "${CMAKE_CURRENT_SOURCE_DIR}/")
  SOURCE_GROUP("Qt Resource Files" FILES ${_ORG_QRC})
  
  SOURCE_GROUP("Plugin META Files" FILES ${_ORG_META})
 
ENDMACRO()

MACRO(_MACRO_APPEND_TO_LIST _list _value)
  SET(_origlist ${${_list}})
  SET(${_list} )
  FOREACH(_element ${_origlist})
    LIST(APPEND ${_list} "${_value}${_element}")
  ENDFOREACH()
ENDMACRO()
    
 