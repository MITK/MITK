# This file is intended to be included at the end of a custom MITKDashboardScript.TEMPLATE.cmake file

list(APPEND CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#
# Automatically determined properties
#
set(MY_OPERATING_SYSTEM "${CMAKE_HOST_SYSTEM}") # Windows 7, Linux-2.6.32, Darwin... 
site_name(CTEST_SITE)

if(QT_BINARY_DIR)
  set(QT_QMAKE_EXECUTABLE "${QT_BINARY_DIR}/qmake")
else()
  set(QT_QMAKE_EXECUTABLE "qmake")
endif()

execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} --version
                OUTPUT_VARIABLE MY_QT_VERSION
                RESULT_VARIABLE qmake_error)
if(qmake_error)
  message(FATAL_ERROR "Error when executing ${QT_QMAKE_EXECUTABLE} --version\n${qmake_error}")
endif()

string(REGEX REPLACE ".*Qt version ([0-9.]+) .*" "\\1" MY_QT_VERSION ${MY_QT_VERSION})

#
# Project specific properties
#
if(NOT CTEST_BUILD_NAME)
  set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM}-${MY_COMPILER}-Qt-${MY_QT_VERSION}-${CTEST_BUILD_CONFIGURATION}")
endif()
set(PROJECT_BUILD_DIR "MITK-build")

set(CTEST_PATH "$ENV{PATH}")
if(WIN32)
  set(VTK_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/VTK-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(ITK_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/ITK-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(BOOST_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/Boost-src/stage/lib")
  set(GDCM_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/GDCM-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(OPENCV_BINARY_DIR "${CTEST_BINARY_DIRECTORY}/OpenCV-build/bin/${CTEST_BUILD_CONFIGURATION}")
  set(BLUEBERRY_OSGI_DIR "${CTEST_BINARY_DIRECTORY}/MITK-build/bin/BlueBerry/org.blueberry.osgi/bin/${CTEST_BUILD_CONFIGURATION}")
  set(CTEST_PATH "${CTEST_PATH};${QT_BINARY_DIR};${VTK_BINARY_DIR};${ITK_BINARY_DIR};${BOOST_BINARY_DIR};${GDCM_BINARY_DIR};${OPENCV_BINARY_DIR};${BLUEBERRY_OSGI_DIR}")
endif()
set(ENV{PATH} "${CTEST_PATH}")

set(SUPERBUILD_TARGETS "")

# If the dashscript doesn't define a GIT_REPOSITORY variable, let's define it here.
if (NOT DEFINED GIT_REPOSITORY OR GIT_REPOSITORY STREQUAL "")
  set(GIT_REPOSITORY "http://git.mitk.org/MITK.git")
endif()

#
# Site specific options
#
if(NOT CDASH_ADMIN_URL_PREFIX)
  set(CDASH_ADMIN_URL_PREFIX "http://mbits")
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
if(CMAKE_GENERATOR MATCHES "[Mm]ake")
  set(CTEST_USE_LAUNCHERS 1)
else()
  set(CTEST_USE_LAUNCHERS 0)
endif()

# Remove this if block after all dartclients work
if(DEFINED ADDITIONNAL_CMAKECACHE_OPTION)
  message(WARNING "Rename ADDITIONNAL to ADDITIONAL in your dartlclient script: ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")
  set(ADDITIONAL_CMAKECACHE_OPTION ${ADDITIONNAL_CMAKECACHE_OPTION})
endif()

if(NOT DEFINED MITK_USE_Boost)
  set(MITK_USE_Boost 1)
endif()

if(NOT DEFINED MITK_USE_OpenCV)
  set(MITK_USE_OpenCV 1)
endif()

set(INITIAL_CMAKECACHE_OPTIONS "
BLUEBERRY_BUILD_TESTING:BOOL=TRUE
BLUEBERRY_BUILD_ALL_PLUGINS:BOOL=TRUE
MITK_BUILD_ALL_PLUGINS:BOOL=TRUE
MITK_BUILD_ALL_APPS:BOOL=TRUE
SUPERBUILD_EXCLUDE_MITKBUILD_TARGET:BOOL=TRUE
MITK_USE_Boost:BOOL=${MITK_USE_Boost}
MITK_USE_OpenCV:BOOL=${MITK_USE_OpenCV}
${ADDITIONAL_CMAKECACHE_OPTION}
")

# Write a cache file for populating the MITK initial cache (not the superbuild cache).
# This can be used to provide variables which are not passed through the
# superbuild process to the MITK configure step)
if(MITK_INITIAL_CACHE)
  set(mitk_cache_file "${CTEST_SCRIPT_DIRECTORY}/mitk_initial_cache.txt")
  file(WRITE "${mitk_cache_file}" "${MITK_INITIAL_CACHE}")
  set(INITIAL_CMAKECACHE_OPTIONS "${INITIAL_CMAKECACHE_OPTIONS}
MITK_INITIAL_CACHE_FILE::INTERNAL=${mitk_cache_file}
")
endif()


#
# Download and include dashboard driver script 
#
if(NOT DEFINED GIT_BRANCH OR GIT_BRANCH STREQUAL "")
  set(hb "HEAD")
else()
  set(hb "refs/heads/${GIT_BRANCH}")
endif()
set(url "http://mitk.org/git/?p=MITK.git;a=blob_plain;f=CMake/MITKDashboardDriverScript.cmake;hb=${hb}")
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile("${url}" "${dest}")
include(${dest})


