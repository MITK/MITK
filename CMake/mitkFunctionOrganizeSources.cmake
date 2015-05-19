function(mitkFunctionOrganizeSources)

  # this functions gets a filelist as input and looks
  # for corresponding h-files to add them to the project.

  # additionally files are grouped in source-groups.

  # No parameters explicitly declared here, because
  # we want to allow for variable argument lists, which
  # are later accessed by the keyword foreach(MYFILE ${ARGV})

  # output: after calling the macro, files that were found
  # correspondigly to the given files are stored in the
  # variable:
  # ${CORRESPONDING_H_FILES}
  # ${CORRESPONDING_TXX_FILES}
  # ${CORRESPONDING_UI_H_FILES}

  # Globbed can be found in the variables
  # ${GLOBBED_TXX_FILES} (CURRENTLY COMMENTED OUT)
  # ${GLOBBED_DOX_FILES}

  cmake_parse_arguments(_ORG "" "" "HEADER;SOURCE;TXX;DOC;MOC;GEN_QRC;GEN_UI;META;UI;QRC" ${ARGN})

  set(CORRESPONDING__H_FILES "")
  set(GLOBBED__H_FILES "")

  if(_ORG_SOURCE)
    foreach(_file ${_ORG_SOURCE})
      string(REGEX REPLACE "(.*)\\.(txx|cpp|c|cxx)$" "\\1.h" H_FILE ${_file})
      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${H_FILE}")
        list(APPEND CORRESPONDING__H_FILES "${H_FILE}")
      endif()
    endforeach()
  else()
    file(GLOB_RECURSE GLOBBED__H_FILES *.h)
  endif()

  set(CORRESPONDING__H_FILES ${CORRESPONDING__H_FILES} PARENT_SCOPE)
  set(GLOBBED__H_FILES ${GLOBBED__H_FILES} PARENT_SCOPE)

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
  source_group("== Header Files ==" FILES ${_ORG_HEADER} ${CORRESPONDING__H_FILES} ${GLOBBED__H_FILES})

  if(_ORG_UI)
    #_MACRO_APPEND_TO_LIST(_ORG_UI "${CMAKE_CURRENT_SOURCE_DIR}/")
    source_group("QT UI Files" FILES ${_ORG_UI})
  endif()

  if(_ORG_DOC)
    #_MACRO_APPEND_TO_LIST(_ORG_DOC "${CMAKE_CURRENT_SOURCE_DIR}/")
    source_group("Doxygen Files" FILES ${_ORG_DOC})
  endif()

  if(_ORG_QRC)
    #_MACRO_APPEND_TO_LIST(_ORG_QRC "${CMAKE_CURRENT_SOURCE_DIR}/")
    source_group("Qt Resource Files" FILES ${_ORG_QRC})
  endif()

  if(_ORG_META)
    source_group("Plugin META Files" FILES ${_ORG_META})
  endif()

endfunction()

macro(_MACRO_APPEND_TO_LIST _list _value)
  set(_origlist ${${_list}})
  set(${_list} )
  foreach(_element ${_origlist})
    list(APPEND ${_list} "${_value}${_element}")
  endforeach()
endmacro()

