#
# OS: Ubuntu 9.04 2.6.28-18-generic
# Hardware: x86_64 GNU/Linux
# GPU: NA
#

# Note: The specific version and processor type of this machine should be reported in the 
# header above. Indeed, this file will be send to the dashboard as a NOTE file. 

cmake_minimum_required(VERSION 2.8.4)

#
# Dashboard properties
#

set(MY_COMPILER "gcc-4.4.5")
# For Windows, e.g.
#set(MY_COMPILER "VC9.0")

set(CTEST_CMAKE_COMMAND "/usr/bin/cmake")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_DASHBOARD_ROOT "/opt/dartclients")
# For Windows, e.g.
#set(CTEST_CMAKE_COMMAND "cmake")
#set(CTEST_CMAKE_GENERATOR "Visual Studio 9 2008 Win64")
#set(CTEST_DASHBOARD_ROOT "C:/dartclients")

# The directory containing the Qt binaries
set(QT_BINARY_DIR "/usr/bin/")
# For Windows, e.g.
#set(QT_BINARY_DIR "V:/windows/x64/QT-4.7.0_VC9.0_Bin/bin")

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
if(UNIX OR MINGW)
  set(CTEST_BUILD_FLAGS "-j4") # Use multiple CPU cores to build
else()
  set(CTEST_BUILD_FLAGS "")
endif()

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

set(ADDITIONAL_CMAKECACHE_OPTION "
# add entries like this
# MITK_USE_OpenCV:BOOL=OFF
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
# The git repository containing MITK code
#set(GIT_REPOSITORY "/home/username/MITK")
# The branch of the MITK git repository to check out
#set(GIT_BRANCH "bug-xxx-label")

##########################################
# WARNING: DO NOT EDIT BEYOND THIS POINT #
##########################################

#
# Convenient macro allowing to download a file
#
macro(downloadFile url dest)
  file(DOWNLOAD "${url}" "${dest}" STATUS status)
  list(GET status 0 error_code)
  list(GET status 1 error_msg)
  if(error_code)
    message(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  endif()
endmacro()

#
# Download and include setup script
#
if(NOT DEFINED GIT_BRANCH OR GIT_BRANCH STREQUAL "")
  set(hb "HEAD")
else()
  set(hb "refs/heads/${GIT_BRANCH}")
endif()
set(url "http://mitk.org/git/?p=MITK.git;a=blob_plain;f=CMake/MITKDashboardSetup.cmake;hb=${hb}")
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.setup)
downloadFile("${url}" "${dest}")
include(${dest})


