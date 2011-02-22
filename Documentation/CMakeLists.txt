# 
# Variables:
#  MITK_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)

OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
OPTION(BUILD_MITK_QCH_FILES "Use doxygen to build Qt creator help files" OFF)
MARK_AS_ADVANCED(USE_DOT)

SET(HAVE_DOT "NO")
IF(DOXYGEN_DOT_EXECUTABLE AND USE_DOT)
  SET(HAVE_DOT "YES")
ENDIF()

SET(MITK_DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Documentation/Doxygen CACHE PATH "Output directory for doxygen generated documentation." )
SET(MITK_HELPPAGES_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Documentation/helpPages CACHE PATH "Output directory for html help pages." )
  
SET(BUILD_QCH_FILES_BOOL "NO")
IF(BUILD_MITK_QCH_FILES)
    SET(QHELPGENERATOR_DIR ${QT_DIR}/bin CACHE PATH "Help generator directory." )
    SET(QCH_OUTPUT_DIR ${MITK_DOXYGEN_OUTPUT_DIR}/QCH CACHE PATH "Help generator directory." )

    #OPTION(BUILD_MITK_QCH_FILE "Use doxygen to build a mitk.qch file" ON)
    OPTION(BUILD_VTK_QCH_FILE "Use doxygen to build a vtk.qch file" OFF)
    OPTION(BUILD_ITK_QCH_FILE "Use doxygen to build a itk.qch file" OFF)

    #ADD_CUSTOM_TARGET(qchfiles
    #             ${DOXYGEN}
    #                 ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf )

    SET(BUILD_QCH_FILES_BOOL "YES")
ENDIF(BUILD_MITK_QCH_FILES)


IF(MITK_USE_BLUEBERRY)
  FILE(RELATIVE_PATH _blueberry_doxygen_path ${MITK_DOXYGEN_OUTPUT_DIR}/html ${BLUEBERRY_DOXYGEN_OUTPUT_DIR}/html)
  SET(BLUEBERRY_TAGFILE "${BLUEBERRY_DOXYGEN_OUTPUT_DIR}/BlueBerry.tag=${_blueberry_doxygen_path}")
  SET(BLUEBERRY_DOXYGEN_LINK "<a class=\"el\" href=\"${_blueberry_doxygen_path}/index.html\">BlueBerry Documentation</a>")
  SET(MITK_XP_LINK "\\ref mitkExtPointsIndex")
  CONFIGURE_FILE(schema.css ${MITK_DOXYGEN_OUTPUT_DIR}/html/schema.css)
  
  SET(MITK_DOXYGEN_ENABLED_SECTIONS "${MITK_DOXYGEN_ENABLED_SECTIONS} BLUEBERRY")
ENDIF(MITK_USE_BLUEBERRY)

# Configure some doxygen options
IF(NOT MITK_DOXYGEN_GENERATE_TODOLIST)
  SET(MITK_DOXYGEN_GENERATE_TODOLIST "NO")
ENDIF()

IF(NOT MITK_DOXYGEN_GENERATE_BUGLIST)
  SET(MITK_DOXYGEN_GENERATE_BUGLIST "NO")
ENDIF()

IF(NOT MITK_DOXYGEN_HTML_DYNAMIC_SECTIONS)
  SET(MITK_DOXYGEN_HTML_DYNAMIC_SECTIONS "NO")
ENDIF()

IF(NOT MITK_DOXYGEN_UML_LOOK)
  SET(MITK_DOXYGEN_UML_LOOK "NO")
ENDIF()

IF(NOT MITK_DOXYGEN_GENERATE_DEPRECATEDLIST)
  SET(MITK_DOXYGEN_GENERATE_DEPRECATEDLIST "YES")
ENDIF()

CONFIGURE_FILE(Doxygen/MainPage.dox.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxygen/MainPage.dox)
CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)
CONFIGURE_FILE(helpPages.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/helpPages.conf)


IF(BUILD_ITK_QCH_FILE)
  # add the command to generate the ITK documantation
  add_custom_command(TARGET doc
                     POST_BUILD
                     ${DOXYGEN}  ${CMAKE_CURRENT_BINARY_DIR}/doxygen.itk.conf)
ENDIF(BUILD_ITK_QCH_FILE)

IF(BUILD_VTK_QCH_FILE)
  # add the command to generate the VTK documantation
  add_custom_command(TARGET doc
                     POST_BUILD
                     ${DOXYGEN}  ${CMAKE_CURRENT_BINARY_DIR}/doxygen.vtk.conf)
ENDIF(BUILD_VTK_QCH_FILE)

# CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/Documentation/doxygen.conf.in
#                ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf)


ADD_CUSTOM_TARGET(doc 
                  ${DOXYGEN} 
                  ${PROJECT_BINARY_DIR}/Documentation/doxygen.conf
                  )

IF (MITK_USE_BLUEBERRY)  
  # convert the extension points schema files into html
  FIND_PACKAGE(Ant)
  IF(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)

    LIST(APPEND MITK_XP_GLOB_EXPRESSIONS
         ${MITK_SOURCE_DIR}/CoreUI/Bundles/plugin.xml
         ${MITK_SOURCE_DIR}/Modules/Bundles/plugin.xml)

    FILE(GLOB_RECURSE _plugin_xmls ${MITK_XP_GLOB_EXPRESSIONS})
                      
    MACRO_CONVERT_SCHEMA(INPUT ${_plugin_xmls} 
                         OUTPUT_DIR "${MITK_DOXYGEN_OUTPUT_DIR}/html/extension-points/html"
                         TARGET_NAME mitkXPDoc
                         )

    ADD_DEPENDENCIES(doc mitkXPDoc)
    IF(${PROJECT_NAME} STREQUAL "MITK")
      ADD_DEPENDENCIES(doc BlueBerryDoc)
    ENDIF()
  ENDIF(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)
ENDIF (MITK_USE_BLUEBERRY)

ADD_CUSTOM_TARGET(helpPages
                  ${DOXYGEN} 
                  ${PROJECT_BINARY_DIR}/Documentation/helpPages.conf
                  )
ELSE(DOXYGEN_FOUND)
# copy blank documentation page to prevent QtHelp from being shown 
# copy the .qhc and .qch files to $MITK_BIN/mitk/bin/ExtBundles/resources/

CONFIGURE_FILE(pregenerated/MITKBlankPage.qch
               ${MITK_BINARY_DIR}/bin/ExtBundles/org.mitk.gui.qt.extapplication/resources/MITKBlankPage.qch
               COPYONLY)

CONFIGURE_FILE(pregenerated/MitkExtQtHelpCollection.qhc
               ${MITK_BINARY_DIR}/bin/ExtBundles/org.mitk.gui.qt.extapplication/resources/MitkExtQtHelpCollection.qhc
               COPYONLY)

ENDIF(DOXYGEN_FOUND)

