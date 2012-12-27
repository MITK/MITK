# this macro gets a filelist as input and looks
# for corresponding h-files to add them to the project.
#
# additionally files are grouped in source-groups.
#
# No parameters explicitly declared here, because
# we want to allow for variable argument lists, which
# are later access by the keyword foreach(_file ${ARGV})
#
# output: after calling the macro, files that were found
# correspondigly to the given files are stored in the
# variable:
# ${CORRESPONDING__H_FILES}
#
# GLOBBED__H_FILES


macro(MACRO_ORGANIZE_SOURCES)

  MACRO_PARSE_ARGUMENTS(_ORG "HEADER;SOURCE;TXX;DOC;MOC;GEN_QRC;GEN_UI;META;UI;QRC" "" ${ARGN})

  set(CORRESPONDING__H_FILES "" )
  set(GLOBBED__H_FILES "" )

  if(_ORG_HEADER)
    foreach(_file ${_ORG_SOURCE})
      string(REGEX REPLACE "(.*)\\.(txx|cpp|c|cxx)$" "\\1.h" H_FILE ${_file})
      if(EXISTS ${H_FILE})
        list(APPEND CORRESPONDING__H_FILES "${H_FILE}")
      endif()
    endforeach()
  else()
    file(GLOB_RECURSE GLOBBED__H_FILES *.h)
  endif()

  if(_ORG_GEN_QRC OR _ORG_GEN_UI OR _ORG_MOC)
    source_group("Generated\\Qt QRC Source Files" FILES ${_ORG_GEN_QRC})
    source_group("Generated\\Qt UI Header Files" FILES ${_ORG_GEN_UI})
    source_group("Generated\\Qt MOC Source Files" FILES ${_ORG_MOC})
  endif()

  #_MACRO_APPEND_TO_LIST(_ORG_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/")
  source_group("== Source Files ==" FILES ${_ORG_SOURCE})

  #_MACRO_APPEND_TO_LIST(_ORG_TXX "${CMAKE_CURRENT_SOURCE_DIR}/")
  source_group("== Template Files ==" FILES ${_ORG_TXX})

  #_MACRO_APPEND_TO_LIST(_ORG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/")
  source_group("== Header Files ==" FILES ${_ORG_HEADER} ${_ORG_HEADER} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES})


  #_MACRO_APPEND_TO_LIST(_ORG_UI "${CMAKE_CURRENT_SOURCE_DIR}/")
  source_group("QT UI Files" FILES ${_ORG_UI})

  #_MACRO_APPEND_TO_LIST(_ORG_DOC "${CMAKE_CURRENT_SOURCE_DIR}/")
  source_group("Doxygen Files" FILES ${_ORG_DOC})

  #_MACRO_APPEND_TO_LIST(_ORG_QRC "${CMAKE_CURRENT_SOURCE_DIR}/")
  source_group("Qt Resource Files" FILES ${_ORG_QRC})

  source_group("Plugin META Files" FILES ${_ORG_META})

endmacro()

macro(_MACRO_APPEND_TO_LIST _list _value)
  set(_origlist ${${_list}})
  set(${_list} )
  foreach(_element ${_origlist})
    list(APPEND ${_list} "${_value}${_element}")
  endforeach()
endmacro()

