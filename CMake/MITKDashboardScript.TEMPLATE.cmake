#
# OS: Ubuntu 9.04 2.6.28-18-generic
# Hardware: x86_64 GNU/Linux
# GPU: NA
#

# Note: The specific version and processor type of this machine should be reported in the 
# header above. Indeed, this file will be send to the dashboard as a NOTE file. 

cmake_minimum_required(VERSION 2.8.2)

#
# Dashboard properties
#
set(MY_COMPILER "g++4.4.5")

set(CTEST_CMAKE_COMMAND "/usr/bin/cmake")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_DASHBOARD_ROOT "/opt/dartclients")

set(QT_QMAKE_EXECUTABLE "/usr/bin/qmake")

#
# Dashboard options
#
set(WITH_KWSTYLE FALSE)
set(WITH_MEMCHECK FALSE)
set(WITH_COVERAGE FALSE)
set(WITH_DOCUMENTATION FALSE)
#set(DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY ) # for example: $ENV{HOME}/Projects/Doxygen
set(CTEST_BUILD_CONFIGURATION "Release")
set(CTEST_TEST_TIMEOUT 500)
set(CTEST_BUILD_FLAGS "-j4") # Use multiple CPU cores to build

# experimental: 
#     - run_ctest() macro will be called *ONE* time
#     - binary directory will *NOT* be cleaned
# continuous: 
#     - run_ctest() macro will be called EVERY 5 minutes ... 
#     - binary directory will *NOT* be cleaned
#     - configure/build will be executed *ONLY* if the repository has been updated
# nightly: 
#     - run_ctest() macro will be called *ONE* time
#     - binary directory *WILL BE* cleaned
set(SCRIPT_MODE "experimental") # "experimental", "continuous", "nightly"

#
# Project specific properties
#
set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MITK")
set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MITK-Superbuild-${CTEST_BUILD_CONFIGURATION}-${SCRIPT_MODE}")

set(ADDITIONNAL_CMAKECACHE_OPTION "
#MITK_USE_Boost:BOOL=ON
")

# List of test that should be explicitly disabled on this machine
set(TEST_TO_EXCLUDE_REGEX "")

# set any extra environment variables here
set(ENV{DISPLAY} ":0")

find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
find_program(CTEST_GIT_COMMAND NAMES git)

#
# Git repository - Overwrite the default value provided by the driver script
#
# set(GIT_REPOSITORY "/home/username/MITK")
# set(GIT_BRANCH "bug-xxx-description")

##########################################
# WARNING: DO NOT EDIT BEYOND THIS POINT #
##########################################

set(CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}")

#
# Automatically determined properties
#
set(MY_OPERATING_SYSTEM "${CMAKE_HOST_SYSTEM}") # Windows 7, Linux-2.6.32, Darwin... 
site_name(CTEST_SITE)

execute_process(COMMAND qmake --version
                OUTPUT_VARIABLE MY_QT_VERSION)
string(REGEX REPLACE ".*Qt version ([0-9.]+) .*" "\\1" MY_QT_VERSION ${MY_QT_VERSION})

#
# Project specific properties
#
set(CTEST_PROJECT_NAME "MITK")
set(CTEST_BUILD_NAME "${MY_OPERATING_SYSTEM}-${MY_COMPILER}-Qt-${MY_QT_VERSION}-${CTEST_BUILD_CONFIGURATION}")

#
# Display build info
#
message("Site name: ${CTEST_SITE}")
message("Build name: ${CTEST_BUILD_NAME}")
message("Script Mode: ${SCRIPT_MODE}")
message("Coverage: ${WITH_COVERAGE}, MemCheck: ${WITH_MEMCHECK}")

#
# Convenient macro allowing to download a file
#
MACRO(downloadFile url dest)
  FILE(DOWNLOAD "${url}" "${dest}" STATUS status)
  LIST(GET status 0 error_code)
  LIST(GET status 1 error_msg)
  IF(error_code)
    MESSAGE(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  ENDIF()
ENDMACRO()

#
# Download and include dashboard driver script 
#
if(NOT DEFINED GIT_BRANCH OR GIT_BRANCH STREQUAL "")
  set(hb "HEAD")
else()
  set(hb "refs/heads/${GIT_BRANCH}")
endif()
set(url "http://mbits/git/?p=MITK.git;a=blob_plain;f=CMake/MITKDashboardDriverScript.cmake;hb=${hb}")
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.driver)
downloadFile("${url}" "${dest}")
INCLUDE(${dest})


