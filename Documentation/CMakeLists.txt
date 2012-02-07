# 
# Variables:
#  MITK_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)

# Compile source code snippets
add_subdirectory(Snippets)

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)

OPTION(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
OPTION(MITK_DOXYGEN_BUILD_ALWAYS "Always build the MITK documentation when building the default target" OFF)
OPTION(MITK_DOXYGEN_GENERATE_QCH_FILES "Use doxygen to generate Qt compressed help files for MITK docs" OFF)
MARK_AS_ADVANCED(USE_DOT MITK_DOXYGEN_BUILD_ALWAYS MITK_DOXYGEN_GENERATE_QCH_FILES)

SET(HAVE_DOT "NO")
IF(DOXYGEN_DOT_EXECUTABLE AND USE_DOT)
  SET(HAVE_DOT "YES")
ENDIF()

SET(MITK_DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Documentation/Doxygen CACHE PATH "Output directory for doxygen generated documentation." )
SET(MITK_DOXYGEN_TAGFILE_NAME ${MITK_DOXYGEN_OUTPUT_DIR}/MITK.tag CACHE INTERNAL "MITK Doxygen tag file")

# This is relative to the working directory of the doxygen command
SET(MITK_DOXYGEN_STYLESHEET mitk_doxygen.css)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${MITK_DOXYGEN_STYLESHEET}
               ${CMAKE_CURRENT_BINARY_DIR}/${MITK_DOXYGEN_STYLESHEET} @COPYONLY)

# Create QCH files for MITK and external projects
SET(MITK_DOXYGEN_GENERATE_QHP "NO")
IF(MITK_DOXYGEN_GENERATE_QCH_FILES)
  
  FIND_PROGRAM(QT_HELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator qhelpgenerator-qt4 qhelpgenerator4
    PATHS ${QT_BINARY_DIR}
    DOC "The location of the the Qt help generator executable"
    NO_DEFAULT_PATH
  )
  MARK_AS_ADVANCED(QT_HELPGENERATOR_EXECUTABLE)
  
  IF(NOT QT_HELPGENERATOR_EXECUTABLE)
    MESSAGE(SEND_ERROR "The Qt help generator could not be found. Disabling qch generation")
  ELSE()
    SET(MITK_DOXYGEN_GENERATE_QHP "YES")
  ENDIF()

  # The name of the generated MITK qch file, relative to the
  # Doxygen HTML output folder
  SET(MITK_DOXYGEN_QCH_FILE "${MITK_BINARY_DIR}/MITK-${MITK_REVISION_ID}.qch")

  # Generating ITK and VTK docs it not done yet
  #OPTION(MITK_DOXYGEN_GENERATE_VTK_QCH_FILE "Use doxygen to generate a Qt compressed help file for VTK docs" OFF)
  #OPTION(MITK_DOXYGEN_GENERATE_ITK_QCH_FILE "Use doxygen to generate a Qt compressed help file for ITK docs" OFF)
  #MARK_AS_ADVANCED(MITK_DOXYGEN_GENERATE_VTK_QCH_FILE MITK_DOXYGEN_GENERATE_ITK_QCH_FILE)

ENDIF()


IF(MITK_USE_BLUEBERRY)
  FILE(RELATIVE_PATH _blueberry_doxygen_path ${MITK_DOXYGEN_OUTPUT_DIR}/html ${BLUEBERRY_DOXYGEN_OUTPUT_DIR}/html)
  SET(BLUEBERRY_DOXYGEN_TAGFILE "${BLUEBERRY_DOXYGEN_TAGFILE_NAME}=${_blueberry_doxygen_path}")
  SET(BLUEBERRY_DOXYGEN_LINK "<a class=\"el\" href=\"${_blueberry_doxygen_path}/index.html\">BlueBerry Documentation</a>")
  SET(MITK_XP_LINK "\\ref mitkExtPointsIndex")
  CONFIGURE_FILE(schema.css ${MITK_DOXYGEN_OUTPUT_DIR}/html/schema.css)
  
  SET(MITK_DOXYGEN_ENABLED_SECTIONS "${MITK_DOXYGEN_ENABLED_SECTIONS} BLUEBERRY")
ENDIF(MITK_USE_BLUEBERRY)

# Compile a doxygen input filter for processing CMake scripts
INCLUDE(mitkFunctionCMakeDoxygenFilterCompile)
mitkFunctionCMakeDoxygenFilterCompile(NAMESPACE "CMake")

# Configure some doxygen options
IF(NOT MITK_DOXYGEN_INTERNAL_DOCS)
  SET(MITK_DOXYGEN_INTERNAL_DOCS "NO")
  SET(MITK_DOXYGEN_HIDE_FRIEND_COMPOUNDS "YES")
  SET(MITK_DOXYGEN_EXCLUDE_PATTERNS "*_p.* *Private.h */internal/*")
ELSE()
  SET(MITK_DOXYGEN_HIDE_FRIEND_COMPOUNDS "NO")
  SET(MITK_DOXYGEN_EXCLUDE_PATTERNS "")
ENDIF()

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

IF(NOT DEFINED MITK_DOXYGEN_DOT_NUM_THREADS)
  SET(MITK_DOXYGEN_DOT_NUM_THREADS 0)
ENDIF()

CONFIGURE_FILE(Doxygen/MainPage.dox.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxygen/MainPage.dox)
CONFIGURE_FILE(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

IF(MITK_DOXYGEN_BUILD_ALWAYS)
  SET(_doc_in_all "ALL")
ELSE()
  SET(_doc_in_all "")
ENDIF()

ADD_CUSTOM_TARGET(doc ${_doc_in_all}
                  ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
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

#IF(MITK_DOXYGEN_GENERATE_ITK_QCH_FILE)
#  # add the command to generate the ITK documantation
#  ADD_CUSTOM_TARGET(doc-itk
#                    COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.itk.conf)
#  ADD_DEPENDENCIES(doc doc-itk)
#ENDIF()

#IF(MITK_DOXYGEN_GENERATE_VTK_QCH_FILE)
#  # add the command to generate the VTK documantation
#  ADD_CUSTOM_TARGET(doc-vtk
#                    COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.vtk.conf)
#  ADD_DEPENDENCIES(doc doc-vtk)
#ENDIF()

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

