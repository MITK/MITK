# 
# Variables:
#  BLUEBERRY_DOCS_OUTPUT_DIR: documentation output directory (optional)

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)

OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOXYGEN_DOT_EXECUTABLE AND USE_DOT)
  SET(HAVE_DOT "YES")
ENDIF()

SET(BLUEBERRY_DOCS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "BlueBerry output directory for all generated documentation.")
SET(BLUEBERRY_DOXYGEN_OUTPUT_DIR ${BLUEBERRY_DOCS_OUTPUT_DIR}/reference/api CACHE INTERNAL "BlueBerry doxygen output directory")
SET(BLUEBERRY_XPDOC_OUTPUT_DIR ${BLUEBERRY_DOCS_OUTPUT_DIR}/reference/extension-points)

FILE(RELATIVE_PATH BLUEBERRY_DOXYGEN_XP_RELATIVEDIR ${BLUEBERRY_DOXYGEN_OUTPUT_DIR}/html ${BLUEBERRY_XPDOC_OUTPUT_DIR}/)

SET(_doxygen_qt4bundles )
SET(_doxygen_bundles )
SET(_doxygen_binary_qt4bundles )
SET(_doxygen_binary_bundles )
FILE(GLOB _bundle_dirs RELATIVE "${BLUEBERRY_PLUGINS_SOURCE_DIR}" "${BLUEBERRY_PLUGINS_SOURCE_DIR}/*")
FOREACH(_bundle_dir ${_bundle_dirs})
  STRING(REGEX MATCH "^\\..*" _matched_hidden "${_bundle_dir}")
  IF(NOT _matched_hidden)
    IF(IS_DIRECTORY "${BLUEBERRY_PLUGINS_SOURCE_DIR}/${_bundle_dir}")
      STRING(REGEX MATCH ".*\\.qt.*" _matched "${_bundle_dir}")
      IF(_matched)
        IF(QT4_FOUND)
          SET(_doxygen_qt4bundles "${_doxygen_qt4bundles} ${BLUEBERRY_PLUGINS_SOURCE_DIR}/${_bundle_dir}")
          SET(_doxygen_binary_qt4bundles  "${_doxygen_binary_qt4bundles} ${BLUEBERRY_PLUGINS_BINARY_DIR}/${_bundle_dir}")
        ENDIF()
      ELSE()
        SET(_doxygen_bundles "${_doxygen_bundles} ${BLUEBERRY_PLUGINS_SOURCE_DIR}/${_bundle_dir}")
        SET(_doxygen_binary_bundles  "${_doxygen_binary_bundles} ${BLUEBERRY_PLUGINS_BINARY_DIR}/${_bundle_dir}")
      ENDIF()
    ENDIF()
  ENDIF()
ENDFOREACH()

CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)
               
CONFIGURE_FILE(reference/extension-points/index.dox.in
               ${CMAKE_CURRENT_BINARY_DIR}/reference/extension-points/index.dox)

FIND_PACKAGE(Doxygen)

ADD_CUSTOM_TARGET(BlueBerryDoc 
                  ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                  DEPENDS ${_BlueBerryDoc_depends}
                  )
                  
# convert the extension points schema files into html
IF(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)
  FILE(GLOB_RECURSE _plugin_xmls  ${BLUEBERRY_PLUGINS_SOURCE_DIR}/plugin.xml)
  MACRO_CONVERT_SCHEMA(INPUT ${_plugin_xmls} 
                       OUTPUT_DIR "${BLUEBERRY_XPDOC_OUTPUT_DIR}"
                       TARGET_NAME BlueBerryXPDoc
                       )

  ADD_DEPENDENCIES(BlueBerryDoc BlueBerryXPDoc)
ENDIF(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)

CONFIGURE_FILE(schema.css ${BLUEBERRY_DOCS_OUTPUT_DIR}/schema.css)

ENDIF(DOXYGEN_FOUND)
