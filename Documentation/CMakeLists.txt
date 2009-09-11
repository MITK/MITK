# 
# Variables:
#  MITK_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)

OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOXYGEN_DOT_EXECUTABLE AND USE_DOT)
  SET(HAVE_DOT "YES")
ENDIF()

SET(MITK_DOXYGEN_OUTPUT_DIR ${MITK_BINARY_DIR}/Documentation/Doxygen CACHE PATH "Output directory for doxygen generated documentation." )
SET(MITK_HELPPAGES_OUTPUT_DIR ${MITK_BINARY_DIR}/Documentation/helpPages CACHE PATH "Output directory for html help pages." )
  

IF(MITK_USE_OPENCHERRY)
  FILE(RELATIVE_PATH _opencherry_doxygen_path ${MITK_DOXYGEN_OUTPUT_DIR}/html ${OPENCHERRY_DOXYGEN_OUTPUT_DIR}/html)
  SET(OPENCHERRY_TAGFILE "${OPENCHERRY_DOXYGEN_OUTPUT_DIR}/openCherry.tag=${_opencherry_doxygen_path}")
  SET(OPENCHERRY_DOXYGEN_LINK "<a class=\"el\" href=\"${_opencherry_doxygen_path}/index.html\">openCherry Documentation</a>")
  SET(MITK_XP_LINK "\\ref mitkExtPointsIndex")
  CONFIGURE_FILE(schema.css ${MITK_DOXYGEN_OUTPUT_DIR}/html/schema.css)
  
  SET(MITK_DOXYGEN_ENABLED_SECTIONS "${MITK_DOXYGEN_ENABLED_SECTIONS} OPENCHERRY")
ENDIF(MITK_USE_OPENCHERRY)

CONFIGURE_FILE(Doxygen/MainPage.dox.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxygen/MainPage.dox)
CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)
CONFIGURE_FILE(helpPages.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/helpPages.conf)

# CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Documentation/doxygen.conf.in
#                ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf)


ADD_CUSTOM_TARGET(doc 
                  ${DOXYGEN} 
                  ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf
                  )

IF (MITK_USE_OPENCHERRY)  
  # convert the extension points schema files into html
  FIND_PACKAGE(Ant)
  IF(ANT_FOUND AND OPENCHERRY_DOC_TOOLS_DIR)
    FILE(GLOB_RECURSE _plugin_xmls  ${CMAKE_CURRENT_SOURCE_DIR}/../Bundles/plugin.xml)
    MACRO_CONVERT_SCHEMA(INPUT ${_plugin_xmls} 
                         OUTPUT_DIR "${MITK_DOXYGEN_OUTPUT_DIR}/html/extension-points/html"
                         TARGET_NAME mitkXPDoc
                         )

    ADD_DEPENDENCIES(doc mitkXPDoc openCherryDoc)
  ENDIF(ANT_FOUND AND OPENCHERRY_DOC_TOOLS_DIR)
ENDIF (MITK_USE_OPENCHERRY)

ADD_CUSTOM_TARGET(helpPages
                  ${DOXYGEN} 
                  ${PROJECT_BINARY_DIR}/Documentation/helpPages.conf
                  )

ENDIF(DOXYGEN_FOUND)

