# 
# Variables:
#  OPENCHERRY_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)

INCLUDE(files.cmake)

OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOT)
  IF(USE_DOT)
    SET(HAVE_DOT "YES")
  ENDIF(USE_DOT)
ENDIF(DOT)


SET(OPENCHERRY_DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/reference/api)

CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

FIND_PACKAGE(Doxygen)

# Doxygen config file requires only the path to DOT, not DOT itself
GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH CACHE)

ADD_CUSTOM_TARGET(openCherryDoc 
                  ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                  DEPENDS ${_openCherryDoc_depends}
                  )
                  
# convert the extension points schema files into html
IF(ANT_FOUND AND OPENCHERRY_DOC_TOOLS_DIR)
  FILE(GLOB_RECURSE _plugin_xmls  ${OPENCHERRY_SOURCE_DIR}/plugin.xml)
  MACRO_CONVERT_SCHEMA(INPUT ${_plugin_xmls} 
                       OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/reference/extension-points"
                       TARGET_NAME openCherryXPDoc
                       )

  ADD_DEPENDENCIES(openCherryDoc openCherryXPDoc)
ENDIF(ANT_FOUND AND OPENCHERRY_DOC_TOOLS_DIR)

FOREACH(_file ${COPY_DOC_FILES})
  CONFIGURE_FILE(${_file} ${CMAKE_CURRENT_BINARY_DIR}/${_file})
ENDFOREACH(_file ${COPY_DOC_FILES})