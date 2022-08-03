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

  find_program(QT_HELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator qhelpgenerator-qt5 qhelpgenerator5
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
