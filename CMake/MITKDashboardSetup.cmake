# This file is intended to be included at the end of a custom MITKDashboardScript.TEMPLATE.cmake file

list(APPEND CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#
# Automatically determined properties
#
set(MY_OPERATING_SYSTEM )

if(UNIX)
  # Download a utility script
  set(url "http://mitk.org/git/?p=MITK.git;a=blob_plain;f=CMake/mitkDetectOS.sh;hb=${hb}")
  set(dest "${CTEST_SCRIPT_DIRECTORY}/mitkDetectOS.sh")
  downloadFile("${url}" "${dest}")
  execute_process(COMMAND sh "${dest}"
  RESULT_VARIABLE _result OUTPUT_VARIABLE _out
  OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT _result)
    set(MY_OPERATING_SYSTEM "${_out}")
  endif()
endif()

if(NOT MY_OPERATING_SYSTEM)
  set(MY_OPERATING_SYSTEM "${CMAKE_HOST_SYSTEM}") # Windows 7, Linux-2.6.32, Darwin...
endif()

site_name(CTEST_SITE)

if(NOT DEFINED MITK_USE_QT)
  set(MITK_USE_QT 1)
endif()

if(MITK_USE_QT)
  if(NOT QT_QMAKE_EXECUTABLE)
    find_program(QT_QMAKE_EXECUTABLE NAMES qmake qmake-qt4
                 HINTS ${QT_BINARY_DIR})
  endif()

  execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} --version
                  OUTPUT_VARIABLE MY_QT_VERSION
                  RESULT_VARIABLE qmake_error)
  if(qmake_error)
    message(FATAL_ERROR "Error when executing ${QT_QMAKE_EXECUTABLE} --version\n${qmake_error}")
  endif()

  string(REGEX REPLACE ".*Qt version ([0-9.]+) .*" "\\1" MY_QT_VERSION ${MY_QT_VERSION})
endif()

#
# Project specific properties
#
if(NOT CTEST_BUILD_NAME)
  if(MITK_USE_QT)
     set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM} ${MY_COMPILER} Qt${MY_QT_VERSION} ${CTEST_BUILD_CONFIGURATION}")
  else()
    set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM} ${MY_COMPILER} ${CTEST_BUILD_CONFIGURATION}")
  endif()
  set(CTEST_BUILD_NAME "${CTEST_BUILD_NAME}${CTEST_BUILD_NAME_SUFFIX}")
endif()
set(PROJECT_BUILD_DIR "MITK-build")

set(CTEST_PATH "$ENV{PATH}")
if(WIN32)
  set(ANN_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/ANN-build/${CTEST_BUILD_CONFIGURATION}")
  set(CPPUNIT_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/CppUnit-build/${CTEST_BUILD_CONFIGURATION}")
  set(GLUT_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/GLUT-build/${CTEST_BUILD_CONFIGURATION}")
  set(GLEW_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/GLEW-build/${CTEST_BUILD_CONFIGURATION}")
  set(TINYXML_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/tinyxml-build/${CTEST_BUILD_CONFIGURATION}")
  set(QWT_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/Qwt-build/${CTEST_BUILD_CONFIGURATION}")
  set(VTK_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/VTK-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(ACVD_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/ACVD-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(ITK_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/ITK-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(BOOST_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/Boost-install/lib")
  set(GDCM_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/GDCM-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(DCMTK_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/DCMTK-install/bin/${CTEST_BUILD_CONFIGURATION}")
  set(OPENCV_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/OpenCV-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(POCO_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/Poco-install/lib")
  set(SOFA_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/SOFA-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(BLUEBERRY_OSGI_DIR "${CTEST_BINARY_DIRECTORY}/MITK-build/bin/BlueBerry/org.blueberry.osgi/bin/${CTEST_BUILD_CONFIGURATION}")
  set(CTEST_PATH "${CTEST_PATH};${CPPUNIT_BINARY_DIR};${QT_BINARY_DIR};${VTK_BINARY_DIR};${ANN_BINARY_DIR};${GLUT_BINARY_DIR};${GLEW_BINARY_DIR};${TINYXML_BINARY_DIR};${QWT_BINARY_DIR};${ACVD_BINARY_DIR};${ITK_BINARY_DIR};${BOOST_BINARY_DIR};${GDCM_BINARY_DIR};${DCMTK_BINARY_DIR};${OPENCV_BINARY_DIR};${POCO_BINARY_DIR};${SOFA_BINARY_DIR};${BLUEBERRY_OSGI_DIR}")
endif()
set(ENV{PATH} "${CTEST_PATH}")

set(SUPERBUILD_TARGETS "")

# If the dashscript doesn't define a GIT_REPOSITORY variable, let's define it here.
if(NOT DEFINED GIT_REPOSITORY OR GIT_REPOSITORY STREQUAL "")
  set(GIT_REPOSITORY "http://git.mitk.org/MITK.git")
endif()

#
# Display build info
#
message("Site name: ${CTEST_SITE}")
message("Build name: ${CTEST_BUILD_NAME}")
message("Script Mode: ${SCRIPT_MODE}")
message("Coverage: ${WITH_COVERAGE}, MemCheck: ${WITH_MEMCHECK}")

#
# Set initial cache options
#
if(${CMAKE_VERSION} VERSION_GREATER "2.8.9")
  set(CTEST_USE_LAUNCHERS 1)
  set(ENV{CTEST_USE_LAUNCHERS_DEFAULT} 1)
endif()

# Remove this if block after all dartclients work
if(DEFINED ADDITIONNAL_CMAKECACHE_OPTION)
  message(WARNING "Rename ADDITIONNAL to ADDITIONAL in your dartlclient script: ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")
  set(ADDITIONAL_CMAKECACHE_OPTION ${ADDITIONNAL_CMAKECACHE_OPTION})
endif()

if(NOT DEFINED MITK_BUILD_CONFIGURATION)
  set(MITK_BUILD_CONFIGURATION "All")
endif()

if(NOT DEFINED MITK_VTK_DEBUG_LEAKS)
  set(MITK_VTK_DEBUG_LEAKS 1)
endif()

set(INITIAL_CMAKECACHE_OPTIONS "
SUPERBUILD_EXCLUDE_MITKBUILD_TARGET:BOOL=TRUE
MITK_BUILD_CONFIGURATION:STRING=${MITK_BUILD_CONFIGURATION}
MITK_VTK_DEBUG_LEAKS:BOOL=${MITK_VTK_DEBUG_LEAKS}
${ADDITIONAL_CMAKECACHE_OPTION}
")

if(MITK_USE_QT)
  set(INITIAL_CMAKECACHE_OPTIONS "${INITIAL_CMAKECACHE_OPTIONS}
QT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}")
endif()

# Write a cache file for populating the MITK initial cache (not the superbuild cache).
# This can be used to provide variables which are not passed through the
# superbuild process to the MITK configure step)
if(MITK_INITIAL_CACHE)
  set(mitk_cache_file "${CTEST_SCRIPT_DIRECTORY}/mitk_initial_cache.txt")
  file(WRITE "${mitk_cache_file}" "${MITK_INITIAL_CACHE}")
  set(INITIAL_CMAKECACHE_OPTIONS "${INITIAL_CMAKECACHE_OPTIONS}
MITK_INITIAL_CACHE_FILE:INTERNAL=${mitk_cache_file}
")
endif()


#
# Download and include dashboard driver script
#
set(url "http://mitk.org/git/?p=MITK.git;a=blob_plain;f=CMake/MITKDashboardDriverScript.cmake;hb=${hb}")
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile("${url}" "${dest}")
include(${dest})


