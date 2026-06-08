# Different doxygen versions produce significantly different behaviour in the MITK documentation
# especially in regards to the MITK Qt assistant help files and markdown files.
# The HTML documentation is supposed to be build with Doxygen 1.8.7 or newer, the
# Qt assistant QCH files are supposed to be generated with Doxygen 1.8.7 or newer.
# So we check for 1.8.7 here and QCH generation support is checked in
# BlueBerry/CMakeLists.txt
set(supported_doxygen_version "1.8.7")

if(DOXYGEN_VERSION VERSION_LESS ${supported_doxygen_version})
  MESSAGE(WARNING "Unsupported doxygen version ${DOXYGEN_VERSION}. The MITK HTML documentation has been tested to work with doxygen ${supported_doxygen_version} or newer.")
endif()

option(USE_DOT "Use dot program for generating graphical class diagrams with doxygen, if available" ON)
option(MITK_DOXYGEN_BUILD_ALWAYS "Always build the MITK documentation when building the default target" OFF)
option(MITK_DOXYGEN_GENERATE_QCH_FILES "Use doxygen to generate Qt compressed help files for MITK docs" OFF)
mark_as_advanced(USE_DOT MITK_DOXYGEN_BUILD_ALWAYS MITK_DOXYGEN_GENERATE_QCH_FILES)
if (MITK_DOXYGEN_GENERATE_QCH_FILES AND DOXYGEN_VERSION VERSION_LESS "1.8.7")
  message(WARNING "> Forcing MITK_DOXYGEN_GENERATE_QCH_FILES to OFF because Doxygen version 1.8.7 or newer not found.")
  set(MITK_DOXYGEN_GENERATE_QCH_FILES OFF CACHE BOOL "Use doxygen to generate Qt compressed help files for MITK docs" FORCE)
endif()

set(HAVE_DOT "NO")
if(DOXYGEN_DOT_EXECUTABLE AND USE_DOT)
  set(HAVE_DOT "YES")
endif()

set(MITK_DOXYGEN_TAGFILE_NAME ${MITK_DOXYGEN_OUTPUT_DIR}/MITK.tag CACHE INTERNAL "MITK Doxygen tag file")

# This is relative to the working directory of the doxygen command
set(MITK_DOXYGEN_STYLESHEET mitk_doxygen_extra.css)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${MITK_DOXYGEN_STYLESHEET}
               ${CMAKE_CURRENT_BINARY_DIR}/${MITK_DOXYGEN_STYLESHEET} COPYONLY)

# Create QCH files for MITK and external projects
set(MITK_DOXYGEN_GENERATE_QHP "NO")
if(MITK_DOXYGEN_GENERATE_QCH_FILES)
  if(NOT QT_HELPGENERATOR_EXECUTABLE)
    message(SEND_ERROR "The Qt help generator could not be found. Disabling qch generation")
  else()
    set(MITK_DOXYGEN_GENERATE_QHP "YES")
  endif()
endif()

# The name of the generated MITK qch file, relative to the
# Doxygen HTML output folder
set(MITK_DOXYGEN_QCH_FILE "${MITK_BINARY_DIR}/MITK-${MITK_REVISION_ID}.qch")

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

if(NOT DEFINED MITK_DOXYGEN_DOT_NUM_THREADS)
  set(MITK_DOXYGEN_DOT_NUM_THREADS 0)
endif()

if(NOT DEFINED US_PLATFORM)
  if(UNIX)
    if(APPLE)
      set(US_PLATFORM "US_PLATFORM_APPLE=1")
    else()
      set(US_PLATFORM "US_PLATFORM_LINUX=1")
    endif()
    set(US_PLATFORM "${US_PLATFORM} \\\nUS_PLATFORM_POSIX=1")
  else()
    set(US_PLATFORM "US_PLATFORM_WINDOWS=1")
  endif()
endif()

# Auto-generate the PREDEFINED list of MITK module export macros so Doxygen
# 1.10+ does not misparse function declarations that use them. See
# mitkFunctionGenerateDoxygenExportMacros.cmake for background.
include(${MITK_SOURCE_DIR}/CMake/mitkFunctionGenerateDoxygenExportMacros.cmake)
mitkFunctionGenerateDoxygenExportMacros(MITK_DOXYGEN_MODULE_EXPORT_MACROS)

# Doxygen 1.10 renamed the legacy "modules" navigation concept to "topics"
# and repurposed type="modules" for C++20 modules. Pick the right tab type
# based on the detected Doxygen version so the API Reference dropdown only
# contains entries that link to existing pages.
if(DOXYGEN_VERSION VERSION_LESS 1.9.8)
  set(MITK_DOXYGEN_GROUPS_TAB
    [[<tab type="modules" visible="yes" title="Groups" intro="Here is a list of all groups:"/>]])
else()
  set(MITK_DOXYGEN_GROUPS_TAB
    [[<tab type="topics" visible="yes" title="Groups" intro="Here is a list of all groups:"/>]])
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/MITKDoxygenLayout.xml.in
               ${CMAKE_CURRENT_BINARY_DIR}/MITKDoxygenLayout.xml
               @ONLY)

# Doxygen renamed the "all groups" index page from modules.html to
# topics.html starting with 1.9.8. Substitute the right name into the
# generated Overview.dox so the inline link in the main page works on
# every supported Doxygen version.
if(DOXYGEN_VERSION VERSION_LESS 1.9.8)
  set(MITK_DOXYGEN_GROUPS_PAGE "modules.html")
else()
  set(MITK_DOXYGEN_GROUPS_PAGE "topics.html")
endif()

# NOTE: Output path is OUTSIDE Documentation/Doxygen/ because that
# subtree is in the Doxyfile EXCLUDE list (it is the OUTPUT_DIRECTORY
# where Doxygen writes its own HTML). Placing the configured file
# directly under Documentation/ keeps it in Doxygen's INPUT scope
# while staying out of the exclude path.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxygen/1-Overview/Overview.dox.in
               ${MITK_BINARY_DIR}/Documentation/Overview.dox
               @ONLY)

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

set_property(TARGET doc PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Documentation")
