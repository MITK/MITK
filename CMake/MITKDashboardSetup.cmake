# This file is intended to be included at the end of a custom MITKDashboardScript.TEMPLATE.cmake file

list(APPEND CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#
# Automatically determined properties
#
set(MY_OPERATING_SYSTEM )

if(UNIX)
  # Download a utility script
  if(IS_PHABRICATOR_URL)
    set(url "https://phabricator.mitk.org/diffusion/MITK/browse/${GIT_BRANCH}/CMake/mitkDetectOS.sh?view=raw")
  else()
    set(url "https://raw.githubusercontent.com/MITK/MITK/master/CMake/mitkDetectOS.sh")
  endif()
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
    find_program(QT_QMAKE_EXECUTABLE NAMES qmake qmake-qt5
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
  if("${CTEST_CMAKE_GENERATOR}" MATCHES ".*Win64")
    set(CMAKE_LIBRARY_ARCHITECTURE x64)
  else()
    set(CMAKE_LIBRARY_ARCHITECTURE x86)
  endif()
  string(SUBSTRING "${MY_COMPILER}" 2 2 vc_version)

  set(OPENCV_BIN_DIR "${CTEST_BINARY_DIRECTORY}/ep/${CMAKE_LIBRARY_ARCHITECTURE}/vc${vc_version}/bin")

  set(SOFA_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/ep/src/SOFA-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(BLUEBERRY_RUNTIME_DIR "${CTEST_BINARY_DIRECTORY}/MITK-build/bin/plugins/${CTEST_BUILD_CONFIGURATION}")

  set(PYTHON_BINARY_DIRS "${CTEST_BINARY_DIRECTORY}/ep/src/CTK-build/CMakeExternals/Install/bin")
  list(APPEND PYTHON_BINARY_DIRS "${CTEST_BINARY_DIRECTORY}/ep/lib/python2.7/bin")

  set(CTEST_PATH "${CTEST_PATH};${CTEST_BINARY_DIRECTORY}/ep/bin;${QT_BINARY_DIR};${SOFA_BINARY_DIR};${BLUEBERRY_RUNTIME_DIR};${OPENCV_BIN_DIR};${PYTHON_BINARY_DIRS}")
endif()
set(ENV{PATH} "${CTEST_PATH}")

# If the dashscript doesn't define a GIT_REPOSITORY variable, let's define it here.
if(NOT DEFINED GIT_REPOSITORY OR GIT_REPOSITORY STREQUAL "")
  set(GIT_REPOSITORY "https://phabricator.mitk.org/diffusion/MITK/mitk.git")
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
if(CTEST_CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  set(CTEST_USE_LAUNCHERS 1)
else()
  set(CTEST_USE_LAUNCHERS 0)
endif()
set(ENV{CTEST_USE_LAUNCHERS_DEFAULT} ${CTEST_USE_LAUNCHERS})

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
if(IS_PHABRICATOR_URL)
  set(url "https://phabricator.mitk.org/diffusion/MITK/browse/${GIT_BRANCH}/CMake/MITKDashboardDriverScript.cmake?view=raw")
else()
  set(url "https://raw.githubusercontent.com/MITK/MITK/master/CMake/MITKDashboardDriverScript.cmake")
endif()
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile("${url}" "${dest}")
include(${dest})


