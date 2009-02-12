# 
# Variables:
#  MITK_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)


OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOT)
  IF(USE_DOT)
    SET(HAVE_DOT "YES")
  ENDIF(USE_DOT)
ENDIF(DOT)

# this is copied from MainApp's CMakeLists.txt and should be refactored out in a MACRO
GET_DIRECTORY_PROPERTY(CACHE_VARS CACHE_VARIABLES)

FOREACH(kit MITK MBIQM MBISB)
  SET(${kit}_FUNCTIONALITY_NAMES_KNOWN "")

FOREACH(entry ${CACHE_VARS})
STRING(REGEX MATCH "^${kit}_KNOWN_FUNCTIONALITY_[A-Za-z0-9]+$" match ${entry})
IF(match)
  STRING(REGEX REPLACE "^${kit}_KNOWN_FUNCTIONALITY_" "" potential_func_name ${match})
  # now check whether a functionality dir is set and exists
  IF(${match})
    IF(IS_DIRECTORY ${${match}}/UserManual)
      IF(BUILD_${potential_func_name} OR BUILD_ALL_FUNCTIONALITIES)
      SET(USERMANUAL_DIRS "${USERMANUAL_DIRS} ${${match}}/UserManual")
      ENDIF(BUILD_${potential_func_name} OR BUILD_ALL_FUNCTIONALITIES)
    ENDIF(IS_DIRECTORY ${${match}}/UserManual)
  ENDIF(${match})
ENDIF(match)
ENDFOREACH(entry)
ENDFOREACH(kit)
SET(MITK_DOXYGEN_OUTPUT_DIR ${MITK_BINARY_DIR}/Documentation/Doxygen CACHE PATH "Output directory for doxygen generated documentation." )
SET(MITK_HELPPAGES_OUTPUT_DIR ${MITK_BINARY_DIR}/Documentation/helpPages CACHE PATH "Output directory for html help pages." )

IF(USE_QT)
  IF(DESIRED_QT_VERSION EQUAL 3)
    #TODO glob org.mitk.gui.qt*
    FILE(GLOB _qt4bundles ${CMAKE_CURRENT_SOURCE_DIR}/../Bundles/org.mitk.gui.qt*)
    SET(QT_DOCUMENTATION_EXCLUDE_DIRS "")
    FOREACH(_qt4bundle ${_qt4bundles})
      SET(QT_DOCUMENTATION_EXCLUDE_DIRS "${QT_DOCUMENTATION_EXCLUDE_DIRS} ${_qt4bundle}")
    ENDFOREACH(_qt4bundle ${_qt4bundles})
    SET(QT_DOCUMENTATION_EXCLUDE_DIRS "${QT_DOCUMENTATION_EXCLUDE_DIRS} ${MITK_BINARY_DIR}/Q4mitk ${MITK_BINARY_DIR}/Q4Applications ${MITK_SOURCE_DIR}/Q4mitk ${MITK_SOURCE_DIR}/Q4Applications")
  ENDIF(DESIRED_QT_VERSION EQUAL 3)
  IF(DESIRED_QT_VERSION EQUAL 4)
    SET(QT_DOCUMENTATION_EXCLUDE_DIRS 
        "${MITK_BINARY_DIR}/Qmitk ${MITK_BINARY_DIR}/QApplications ${MITK_SOURCE_DIR}/Qmitk ${MITK_SOURCE_DIR}/QApplications ${MITK_SOURCE_DIR}/QFunctionalities ${MITK_BINARY_DIR}/QFunctionalities")
  ENDIF(DESIRED_QT_VERSION EQUAL 4)
ENDIF(USE_QT)

#IF(BUILD_OPENCHERRY)
#  SET(OPENCHERRY_TAGFILE "${OPENCHERRY_DOXYGEN_OUTPUT_DIR}/openCherry.tag=${OPENCHERRY_DOXYGEN_OUTPUT_DIR}/html")
#  SET(OPENCHERRY_DOXYGEN_LINK "<a class=\"el\" href=\"../../../Utilities/openCherry/Documentation/reference/api/html/index.html\">openCherry Documentation</a>")
#  SET(MITK_XP_LINK "\\ref mitkExtPointsIndex")
#  CONFIGURE_FILE(schema.css ${CMAKE_CURRENT_BINARY_DIR}/schema.css)
#ENDIF(BUILD_OPENCHERRY)

CONFIGURE_FILE(Doxygen/MainPage.dox.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxygen/MainPage.dox)
CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)
CONFIGURE_FILE(helpPages.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/helpPages.conf)

# CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Documentation/doxygen.conf.in
#                ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf)
FIND_PACKAGE(Doxygen)

# Doxygen config file requires only the path to DOT, not DOT itself
GET_FILENAME_COMPONENT(DOT_PATH ${DOT} PATH CACHE)

ADD_CUSTOM_TARGET(doc 
                  ${DOXYGEN} 
                  ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf
                  )

#IF (BUILD_OPENCHERRY)  
#  # convert the extension points schema files into html
#  FIND_PACKAGE(Ant)
#  IF(ANT_FOUND)
#    FILE(GLOB_RECURSE _plugin_xmls  ${CMAKE_CURRENT_SOURCE_DIR}/../Bundles/plugin.xml)
#    MACRO_CONVERT_SCHEMA(INPUT ${_plugin_xmls} 
#                         OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Doxygen/extension-points"
#                         TARGET_NAME mitkXPDoc
#                         )
#
#    ADD_DEPENDENCIES(doc mitkXPDoc openCherryDoc)
#  ENDIF(ANT_FOUND)
#ENDIF (BUILD_OPENCHERRY)

ADD_CUSTOM_TARGET(helpPages
                  ${DOXYGEN} 
                  ${PROJECT_BINARY_DIR}/Documentation/helpPages.conf
                  )

# make sure the main app is built already so we get documentation for 
# the Qt generated files 
IF(DESIRED_QT_VERSION EQUAL 3)
ADD_DEPENDENCIES(doc MainApp)
ADD_DEPENDENCIES(helpPages MainApp)
ENDIF(DESIRED_QT_VERSION EQUAL 3)


