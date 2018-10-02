#
# OS: Ubuntu 9.04 2.6.28-18-generic
# Hardware: x86_64 GNU/Linux
# GPU: NA
#

# Note: The specific version and processor type of this machine should be reported in the
# header above. Indeed, this file will be send to the dashboard as a NOTE file.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

#
# Dashboard properties
#

set(MY_COMPILER "gcc-4.9.x")
# For Windows, e.g.
#set(MY_COMPILER "VC12.0")

set(CTEST_CMAKE_COMMAND "/usr/bin/cmake")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_DASHBOARD_ROOT "/opt/dartclients")
# For Windows, e.g.
#set(CTEST_CMAKE_COMMAND "cmake")
#set(CTEST_CMAKE_GENERATOR "Visual Studio 12 2013 Win64")
#set(CTEST_DASHBOARD_ROOT "C:/dartclients")

# The directory containing the Qt binaries
set(QT5_INSTALL_PREFIX "/home/user/Qt/5.11.1/gcc_64")
# For Windows, e.g.
#set(QT5_INSTALL_PREFIX "C:/Qt/5.11.1/msvc2013_64")

set(QT_BINARY_DIR "${QT5_INSTALL_PREFIX}/bin")

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
if(UNIX)
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
# In order to shorten the global path length, the build directory for each DartClient
# uses the following abrevation sceme:
# For build configuration:
# Debug -> d
# Release -> r
# For scripte mode:
# continuous -> c
# nightly -> n
# experimental -> e
# Example directory: /MITK-sb-d-n/ for a nightly MITK superbuild in debug mode.
set(short_of_ctest_build_configuration "")
set(short_of_script_mode "")
string(SUBSTRING ${CTEST_BUILD_CONFIGURATION} 0 1 short_of_ctest_build_configuration)
string(SUBSTRING ${SCRIPT_MODE} 0 1 short_of_script_mode)
set(CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MITK")
set(CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/MITK-sb-${short_of_ctest_build_configuration}-${short_of_script_mode}")

# Create an initial cache file for MITK. This file is used to configure the MITK-Build. Use ADDITIONAL_CMAKECACHE_OPTION
# to configure the MITK-Superbuild. The
set(MITK_INITIAL_CACHE "
# Example how to set a boolean variable in the MITK-Build via this script:
#SET(MITK_ENABLE_TOF_HARDWARE \"TRUE\" CACHE INTERNAL \"Enable ToF Hardware\")
# Example how to set a path variable in the MITK-Build via this script:
#SET(MITK_PMD_LIB \"/home/kilgus/thomas/PMDSDK2/Linux_x86_64/bin/libpmdaccess2.so\" CACHE INTERNAL \"PMD lib\")
")

set(ADDITIONAL_CMAKECACHE_OPTION "
# Superbuild variables are not passed through to the MITK-Build (or any other build like ITK, VTK, ...)
# Use the MITK_INITIAL_CACHE the pass variables to the MITK-Build.
# add entries like this
#MITK_USE_OpenCV:BOOL=OFF
CMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
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
  set(IS_PHABRICATOR_URL FALSE)
  set(url "https://raw.githubusercontent.com/MITK/MITK/master/CMake/MITKDashboardSetup.cmake")
else()
  set(IS_PHABRICATOR_URL TRUE)
  string(REPLACE "/" "%252F" GIT_BRANCH_URL ${GIT_BRANCH})
  set(url "https://phabricator.mitk.org/source/mitk/browse/${GIT_BRANCH_URL}/CMake/MITKDashboardSetup.cmake?view=raw")
endif()
set(dest ${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}.setup)
downloadFile("${url}" "${dest}")
include(${dest})
