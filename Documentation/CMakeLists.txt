# 
# Variables:
#  MITK_DOXYGEN_OUTPUT_DIR: doxygen output directory (optional)

# Compile source code snippets
#add_subdirectory(Snippets)

find_package(Doxygen)

if(DOXYGEN_FOUND)

# Different doxygen versions produce different significantly different behaviour in the MITK documentation
# especially in regards to the MITK Qt assistant help files. For this reason we support only one version at a time
set(supported_doxygen_version "1.8.0")

if(CMAKE_VERSION VERSION_LESS "2.8.8")
  EXECUTE_PROCESS(COMMAND ${DOXYGEN_EXECUTABLE} "--version" OUTPUT_VARIABLE DOXYGEN_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT DOXYGEN_VERSION VERSION_EQUAL ${supported_doxygen_version})
  MESSAGE("Unsupported doxygen version ${DOXYGEN_VERSION}. MITK documentation has been tested to work with doxygen ${supported_doxygen_version}")
endif()

option(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
option(MITK_DOXYGEN_BUILD_ALWAYS "Always build the MITK documentation when building the default target" OFF)
option(MITK_DOXYGEN_GENERATE_QCH_FILES "Use doxygen to generate Qt compressed help files for MITK docs" OFF)
mark_as_advanced(USE_DOT MITK_DOXYGEN_BUILD_ALWAYS MITK_DOXYGEN_GENERATE_QCH_FILES)

set(HAVE_DOT "NO")
if(DOXYGEN_DOT_EXECUTABLE AND USE_DOT)
  set(HAVE_DOT "YES")
endif()

set(MITK_DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Documentation/Doxygen CACHE PATH "Output directory for doxygen generated documentation." )
set(MITK_DOXYGEN_TAGFILE_NAME ${MITK_DOXYGEN_OUTPUT_DIR}/MITK.tag CACHE INTERNAL "MITK Doxygen tag file")

# This is relative to the working directory of the doxygen command
set(MITK_DOXYGEN_STYLESHEET mitk_doxygen.css)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${MITK_DOXYGEN_STYLESHEET}
               ${CMAKE_CURRENT_BINARY_DIR}/${MITK_DOXYGEN_STYLESHEET} @COPYONLY)

# Create QCH files for MITK and external projects
set(MITK_DOXYGEN_GENERATE_QHP "NO")
if(MITK_DOXYGEN_GENERATE_QCH_FILES)
  
  find_program(QT_HELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator qhelpgenerator-qt4 qhelpgenerator4
    PATHS ${QT_BINARY_DIR}
    DOC "The location of the the Qt help generator executable"
    NO_DEFAULT_PATH
  )
  mark_as_advanced(QT_HELPGENERATOR_EXECUTABLE)
  
  if(NOT QT_HELPGENERATOR_EXECUTABLE)
    message(SEND_ERROR "The Qt help generator could not be found. Disabling qch generation")
  else()
    set(MITK_DOXYGEN_GENERATE_QHP "YES")
  endif()

  # The name of the generated MITK qch file, relative to the
  # Doxygen HTML output folder
  set(MITK_DOXYGEN_QCH_FILE "${MITK_BINARY_DIR}/MITK-${MITK_REVISION_ID}.qch")

  # Generating ITK and VTK docs it not done yet
  #option(MITK_DOXYGEN_GENERATE_VTK_QCH_FILE "Use doxygen to generate a Qt compressed help file for VTK docs" OFF)
  #option(MITK_DOXYGEN_GENERATE_ITK_QCH_FILE "Use doxygen to generate a Qt compressed help file for ITK docs" OFF)
  #mark_as_advanced(MITK_DOXYGEN_GENERATE_VTK_QCH_FILE MITK_DOXYGEN_GENERATE_ITK_QCH_FILE)

endif()


if(MITK_USE_BLUEBERRY)
  file(RELATIVE_PATH _blueberry_doxygen_path ${MITK_DOXYGEN_OUTPUT_DIR}/html ${BLUEBERRY_DOXYGEN_OUTPUT_DIR}/html)
  set(BLUEBERRY_DOXYGEN_TAGFILE "${BLUEBERRY_DOXYGEN_TAGFILE_NAME}=${_blueberry_doxygen_path}")
  set(BLUEBERRY_DOXYGEN_LINK "<a class=\"el\" href=\"${_blueberry_doxygen_path}/index.html\">BlueBerry Documentation</a>")
  set(MITK_XP_LINK "\\ref mitkExtPointsIndex")
  configure_file(schema.css ${MITK_DOXYGEN_OUTPUT_DIR}/html/schema.css)
  
  set(MITK_DOXYGEN_ENABLED_SECTIONS "${MITK_DOXYGEN_ENABLED_SECTIONS} BLUEBERRY")
endif(MITK_USE_BLUEBERRY)

# Compile a doxygen input filter for processing CMake scripts
include(mitkFunctionCMakeDoxygenFilterCompile)
mitkFunctionCMakeDoxygenFilterCompile(NAMESPACE "CMake")

# Configure some doxygen options
if(NOT MITK_DOXYGEN_INTERNAL_DOCS)
  set(MITK_DOXYGEN_INTERNAL_DOCS "NO")
  set(MITK_DOXYGEN_HIDE_FRIEND_COMPOUNDS "YES")
  set(MITK_DOXYGEN_EXCLUDE_PATTERNS "*_p.* *Private.h */internal/*")
else()
  set(MITK_DOXYGEN_HIDE_FRIEND_COMPOUNDS "NO")
  set(MITK_DOXYGEN_EXCLUDE_PATTERNS "")
endif()

if(NOT MITK_DOXYGEN_GENERATE_TODOLIST)
  set(MITK_DOXYGEN_GENERATE_TODOLIST "NO")
endif()

if(NOT MITK_DOXYGEN_GENERATE_BUGLIST)
  set(MITK_DOXYGEN_GENERATE_BUGLIST "NO")
endif()

if(NOT MITK_DOXYGEN_HTML_DYNAMIC_SECTIONS)
  set(MITK_DOXYGEN_HTML_DYNAMIC_SECTIONS "NO")
endif()

if(NOT MITK_DOXYGEN_UML_LOOK)
  set(MITK_DOXYGEN_UML_LOOK "NO")
endif()

if(NOT MITK_DOXYGEN_GENERATE_DEPRECATEDLIST)
  set(MITK_DOXYGEN_GENERATE_DEPRECATEDLIST "YES")
endif()

if(NOT DEFINED MITK_DOXYGEN_DOT_NUM_THREADS)
  set(MITK_DOXYGEN_DOT_NUM_THREADS 0)
endif()

configure_file(Doxygen/MainPage.dox.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxygen/MainPage.dox)
configure_file(doxygen.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

if(MITK_DOXYGEN_BUILD_ALWAYS)
  set(_doc_in_all "ALL")
else()
  set(_doc_in_all "")
endif()

add_custom_target(doc ${_doc_in_all}
                  ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  )

if(MITK_USE_BLUEBERRY)  
  # convert the extension points schema files into html
  find_package(Ant)
  if(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)

    list(APPEND MITK_XP_GLOB_EXPRESSIONS
         ${MITK_SOURCE_DIR}/Plugins/plugin.xml)

    file(GLOB_RECURSE _plugin_xmls ${MITK_XP_GLOB_EXPRESSIONS})
                      
    MACRO_CONVERT_SCHEMA(INPUT ${_plugin_xmls} 
                         OUTPUT_DIR "${MITK_DOXYGEN_OUTPUT_DIR}/html/extension-points/html"
                         TARGET_NAME mitkXPDoc
                         )

    add_dependencies(doc mitkXPDoc)
    if(${PROJECT_NAME} STREQUAL "MITK")
      add_dependencies(doc BlueBerryDoc)
    endif()
  endif(ANT_FOUND AND BLUEBERRY_DOC_TOOLS_DIR)
endif(MITK_USE_BLUEBERRY)

#if(MITK_DOXYGEN_GENERATE_ITK_QCH_FILE)
#  # add the command to generate the ITK documantation
#  add_custom_target(doc-itk
#                    COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.itk.conf)
#  add_dependencies(doc doc-itk)
#endif()

#if(MITK_DOXYGEN_GENERATE_VTK_QCH_FILE)
#  # add the command to generate the VTK documantation
#  add_custom_target(doc-vtk
#                    COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.vtk.conf)
#  add_dependencies(doc doc-vtk)
#endif()

else(DOXYGEN_FOUND)

# copy blank documentation page to prevent QtHelp from being shown 
# copy the .qhc and .qch files to $MITK_BIN/mitk/bin/ExtBundles/resources/

configure_file(pregenerated/MITKBlankPage.qch
               ${MITK_BINARY_DIR}/bin/ExtBundles/org.mitk.gui.qt.extapplication/resources/MITKBlankPage.qch
               COPYONLY)

configure_file(pregenerated/MitkExtQtHelpCollection.qhc
               ${MITK_BINARY_DIR}/bin/ExtBundles/org.mitk.gui.qt.extapplication/resources/MitkExtQtHelpCollection.qhc
               COPYONLY)

endif(DOXYGEN_FOUND)

