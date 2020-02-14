# This file is intended to be included at the end of a custom MITKDashboardScript.TEMPLATE.cmake file

list(APPEND CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#
# Automatically determined properties
#
set(MY_OPERATING_SYSTEM )

if(UNIX)
  # Download a utility script
  # See T24757.
  # if(IS_PHABRICATOR_URL)
  #   set(url "https://phabricator.mitk.org/source/mitk/browse/${GIT_BRANCH}/CMake/mitkDetectOS.sh?view=raw")
  # else()
    set(url "https://raw.githubusercontent.com/MITK/MITK/master/CMake/mitkDetectOS.sh")
  # endif()
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

if(NOT DEFINED MITK_USE_Qt5)
  set(MITK_USE_Qt5 1)
endif()

#
# Project specific properties
#
if(NOT CTEST_BUILD_NAME)
  if(MITK_USE_Qt5)
     set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM} ${MY_COMPILER} ${CTEST_BUILD_CONFIGURATION}")
  else()
    set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM} ${MY_COMPILER} ${CTEST_BUILD_CONFIGURATION}")
  endif()
  set(CTEST_BUILD_NAME "${CTEST_BUILD_NAME}${CTEST_BUILD_NAME_SUFFIX}")
endif()
set(PROJECT_BUILD_DIR "MITK-build")

set(CTEST_PATH "$ENV{PATH}")
if(WIN32)
  if("${CTEST_CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    set(CMAKE_LIBRARY_ARCHITECTURE x64)
  else()
    set(CMAKE_LIBRARY_ARCHITECTURE x86)
  endif()
  string(SUBSTRING "${MY_COMPILER}" 2 2 vc_version)

  set(OPENCV_BIN_DIR "${CTEST_BINARY_DIRECTORY}/ep/${CMAKE_LIBRARY_ARCHITECTURE}/vc${vc_version}/bin")

  set(BLUEBERRY_RUNTIME_DIR "${CTEST_BINARY_DIRECTORY}/MITK-build/bin/plugins/${CTEST_BUILD_CONFIGURATION}")

  set(PYTHON_BINARY_DIRS "${CTEST_BINARY_DIRECTORY}/ep/src/CTK-build/CMakeExternals/Install/bin")
  get_filename_component(_python_dir "${Python3_EXECUTABLE}" DIRECTORY)
  list(APPEND PYTHON_BINARY_DIRS "${_python_dir}")

  set(CTEST_PATH "${CTEST_PATH};${CTEST_BINARY_DIRECTORY}/ep/bin;${BLUEBERRY_RUNTIME_DIR};${OPENCV_BIN_DIR};${PYTHON_BINARY_DIRS}")
endif()
set(ENV{PATH} "${CTEST_PATH}")

# If the dashscript doesn't define a GIT_REPOSITORY variable, let's define it here.
if(NOT DEFINED GIT_REPOSITORY OR GIT_REPOSITORY STREQUAL "")
  set(GIT_REPOSITORY "https://phabricator.mitk.org/source/mitk.git")
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

if(MITK_EXTENSIONS)
  set(MITK_EXTENSION_DIRS "")
  foreach(extension ${MITK_EXTENSIONS})
    if(extension MATCHES "[^|]+\\|[^|]+\\|(.+)")
      if(MITK_EXTENSION_DIRS)
        set(MITK_EXTENSION_DIRS "${MITK_EXTENSION_DIRS};")
      endif()
      set(MITK_EXTENSION_DIRS "${MITK_EXTENSION_DIRS}${CTEST_DASHBOARD_ROOT}/${CMAKE_MATCH_1}")
    endif()
  endforeach()
endif()

#
# Download and include dashboard driver script
#
if(IS_PHABRICATOR_URL)
  string(REPLACE "/" "%252F" GIT_BRANCH_URL ${GIT_BRANCH})
  set(url "https://phabricator.mitk.org/source/mitk/browse/${GIT_BRANCH_URL}/CMake/MITKDashboardDriverScript.cmake?view=raw")
else()
  set(url "https://raw.githubusercontent.com/MITK/MITK/master/CMake/MITKDashboardDriverScript.cmake")
endif()
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile("${url}" "${dest}")
include(${dest})
