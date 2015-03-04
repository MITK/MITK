set(MITK_CMAKE_MINIMUM_REQUIRED_VERSION 3.1)
cmake_minimum_required(VERSION ${MITK_CMAKE_MINIMUM_REQUIRED_VERSION})

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

set(project_policies )
foreach(policy ${project_policies})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Superbuild Option - Enabled by default
#-----------------------------------------------------------------------------

option(MITK_USE_SUPERBUILD "Build MITK and the projects it depends on via SuperBuild.cmake." ON)

if(MITK_USE_SUPERBUILD)
  project(MITK-superbuild)
  set(MITK_SOURCE_DIR ${PROJECT_SOURCE_DIR})
  set(MITK_BINARY_DIR ${PROJECT_BINARY_DIR})
else()
  project(MITK VERSION 2014.10.99)
endif()

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(MITK_CMAKE_DIR ${MITK_SOURCE_DIR}/CMake)

set(CMAKE_MODULE_PATH
  ${MITK_CMAKE_DIR}
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# CMake function(s) and macro(s)
#-----------------------------------------------------------------------------

# Standard CMake macros
include(FeatureSummary)
include(CTestUseLaunchers)
include(CMakeParseArguments)
include(FindPackageHandleStandardArgs)

# MITK macros
include(mitkFunctionGetGccVersion)
include(mitkFunctionCheckCompilerFlags)
include(mitkFunctionSuppressWarnings) # includes several functions
include(mitkMacroEmptyExternalProject)
include(mitkFunctionGenerateProjectXml)
include(mitkFunctionEnableBuildConfiguration)
include(mitkFunctionWhitelists)
include(mitkFunctionAddExternalProject)

SUPPRESS_VC_DEPRECATED_WARNINGS()

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
               STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# Check miminum compiler versions
#-----------------------------------------------------------------------------

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # require at least gcc 4.6 as provided by Ubuntu 12.04
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
    message(FATAL_ERROR "GCC version must be at least 4.6")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # require at least clang 3.2 for correct RTTI handling and C++11 support
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
    message(FATAL_ERROR "Clang version must be at least 3.2")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # require at least clang 5.0
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "Apple Clang version must be at least 5.0")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # require at least Visual Studio 2010 (msvc ...)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
    message(FATAL_ERROR "Micorsoft Visual Studio 2010 or newer required (MSVC 16.0)")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang (Linux or Apple), GCC and MSVC.")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  mitkFunctionGetGccVersion(${CMAKE_CXX_COMPILER} GCC_VERSION)
else()
  set(GCC_VERSION 0)
endif()

set(MITK_CXX_STANDARD 11)

# help keeping cross-platform compatibility
# CMake 3.1 does not yet know how to handle AppleClang...
# See http://public.kitware.com/Bug/view.php?id=15355
if(NOT MSVC AND
   NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND
   GCC_VERSION VERSION_GREATER 4.6)
  set(CMAKE_CXX_EXTENSIONS 0)
  set(CMAKE_CXX_STANDARD ${MITK_CXX_STANDARD})
  set(CMAKE_CXX_STANDARD_REQUIRED 1)
endif()

# This is necessary to avoid problems with compile feature checks.
# CMAKE_CXX_STANDARD seems to only set the -std=c++11 flag for targets.
# However, compile flag checks also need to be done with -std=c++11.
# The MITK_CXX11_FLAG variable is also used for external projects
# build during the MITK super-build.
mitkFunctionCheckCompilerFlags("-std=c++11" MITK_CXX11_FLAG)
if(NOT MITK_CXX11_FLAG)
  # Older gcc compilers use -std=c++0x
  mitkFunctionCheckCompilerFlags("-std=c++0x" MITK_CXX11_FLAG)
endif()

#-----------------------------------------------------------------------------
# Warn if source or build path is too long
#-----------------------------------------------------------------------------

if(WIN32)
  set(_src_dir_length_max 50)
  set(_bin_dir_length_max 50)
  if(MITK_USE_SUPERBUILD)
    set(_src_dir_length_max 43) # _src_dir_length_max - strlen(ITK-src)
    set(_bin_dir_length_max 40) # _bin_dir_length_max - strlen(MITK-build)
  endif()

  string(LENGTH "${MITK_SOURCE_DIR}" _src_n)
  string(LENGTH "${MITK_BINARY_DIR}" _bin_n)

  # The warnings should be converted to errors
  if(_src_n GREATER _src_dir_length_max)
    message(WARNING "MITK source code directory path length is too long (${_src_n} > ${_src_dir_length_max})."
                    "Please move the MITK source code directory to a directory with a shorter path." )
  endif()
  if(_bin_n GREATER _bin_dir_length_max)
    message(WARNING "MITK build directory path length is too long (${_bin_n} > ${_bin_dir_length_max})."
                    "Please move the MITK build directory to a directory with a shorter path." )
  endif()
endif()

#-----------------------------------------------------------------------------
# Additional MITK Options (also shown during superbuild)
#-----------------------------------------------------------------------------

macro(env_option name doc value)
  set(_value $ENV{${name}})
  if("${_value}" STREQUAL "")
    set(_value ${value})
  endif()
  option(${name} "${doc}" ${_value})
endmacro()

# -----------------------------------------
# General build options
option(BUILD_SHARED_LIBS "Build MITK with shared libraries" ON)
option(WITH_COVERAGE "Enable/Disable coverage" OFF)
option(BUILD_TESTING "Test the project" ON)

env_option(MITK_BUILD_ALL_APPS "Build all MITK applications" OFF)
env_option(MITK_BUILD_EXAMPLES "Build the MITK Examples" OFF)
option(MITK_ENABLE_PIC_READER "Enable support for reading the DKFZ pic file format." ON)

mark_as_advanced(MITK_BUILD_ALL_APPS
                 MITK_ENABLE_PIC_READER
                )

# -----------------------------------------
# Qt version related variables

if(APPLE)
  set(DESIRED_QT_VERSION 4 CACHE STRING "Pick a version of Qt to use: 4 or 5")
else()
  set(DESIRED_QT_VERSION 5 CACHE STRING "Pick a version of Qt to use: 4 or 5")
endif()

env_option(MITK_USE_QT "Use the Qt Company's Qt library" ON)
set(MITK_DESIRED_QT_VERSION ${DESIRED_QT_VERSION})

if(MITK_USE_QT)
  # find the package at the very beginning, so that QT4_FOUND is available
  if(DESIRED_QT_VERSION MATCHES 4)
    set(MITK_QT4_MINIMUM_VERSION 4.7)
    find_package(Qt4 ${MITK_QT4_MINIMUM_VERSION} REQUIRED)
    set(MITK_USE_Qt4 TRUE)
    set(MITK_USE_Qt5 FALSE)
  endif()
  if(DESIRED_QT_VERSION MATCHES 5)
    set(MITK_QT5_MINIMUM_VERSION 5.0.0)
    set(MITK_USE_Qt4 FALSE)
    set(MITK_USE_Qt5 TRUE)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE PATH "")
    set(MITK_QT5_COMPONENTS Concurrent OpenGL PrintSupport Script Sql Svg WebKitWidgets Xml XmlPatterns UiTools)
    find_package(Qt5 ${MITK_QT5_MINIMUM_VERSION} COMPONENTS ${MITK_QT5_COMPONENTS} REQUIRED)
  endif()
else()
  set(MITK_USE_Qt4 FALSE)
  set(MITK_USE_Qt5 FALSE)
endif()

# ------------------------------------------------------------------------
# Register external projects which can be build with the MITK superbuild
# system. Each mitkFunctionAddExternalProject() call registers an external
# project for which a CMakeExternals/<project>.cmake file must exist. The
# call also creates a MITK_USE_<project> variable (appearing in the CMake
# UI if the NO_CACHE option is *not* given).


# -----------------------------------------
# Optional external projects with no
# inter-dependencies

set_property(GLOBAL PROPERTY MITK_EXTERNAL_PROJECTS "")

mitkFunctionAddExternalProject(NAME Poco        ON  COMPONENTS Foundation Util XML Zip)
mitkFunctionAddExternalProject(NAME Boost       OFF DOC "Use the Boost C++ library")
mitkFunctionAddExternalProject(NAME DCMTK       ON  DOC "EXPERIMENTAL, superbuild only: Use DCMTK in MITK")
mitkFunctionAddExternalProject(NAME OpenIGTLink OFF)

mitkFunctionAddExternalProject(NAME tinyxml     ON  ADVANCED)
mitkFunctionAddExternalProject(NAME GDCM        ON  ADVANCED)
mitkFunctionAddExternalProject(NAME GLUT        OFF ADVANCED)
mitkFunctionAddExternalProject(NAME Raptor2     OFF ADVANCED)
mitkFunctionAddExternalProject(NAME Eigen       ON  ADVANCED DOC "Use the Eigen library")
mitkFunctionAddExternalProject(NAME GLEW        ON  ADVANCED DOC "Use the GLEW library")
mitkFunctionAddExternalProject(NAME ANN         ON  ADVANCED DOC "Use Approximate Nearest Neighbor Library")
mitkFunctionAddExternalProject(NAME CppUnit     ON  ADVANCED DOC "Use CppUnit for unit tests")

mitkFunctionAddExternalProject(NAME PCRE        OFF ADVANCED NO_PACKAGE)
mitkFunctionAddExternalProject(NAME ZLIB        OFF ADVANCED NO_PACKAGE NO_CACHE)

# -----------------------------------------
# The following external projects must be
# ordered according to their
# inter-dependencies

mitkFunctionAddExternalProject(NAME SWIG      OFF ADVANCED NO_PACKAGE DEPENDS PCRE)
mitkFunctionAddExternalProject(NAME Python    OFF          NO_PACKAGE DEPENDS SWIG DOC "Use Python wrapping in MITK")
mitkFunctionAddExternalProject(NAME Numpy     OFF ADVANCED NO_PACKAGE)
mitkFunctionAddExternalProject(NAME OpenCV    OFF)

# These are "hard" dependencies and always set to ON
mitkFunctionAddExternalProject(NAME ITK       ON           NO_CACHE)
mitkFunctionAddExternalProject(NAME VTK       ON           NO_CACHE)

mitkFunctionAddExternalProject(NAME SimpleITK OFF          DEPENDS ITK GDCM SWIG)
mitkFunctionAddExternalProject(NAME ACVD      OFF          DOC "Use Approximated Centroidal Voronoi Diagrams")
mitkFunctionAddExternalProject(NAME CTK       ON           DEPENDS QT DCMTK DOC "Use CTK in MITK")
mitkFunctionAddExternalProject(NAME Rasqal    OFF          DEPENDS Raptor2 PCRE ADVANCED)
mitkFunctionAddExternalProject(NAME Redland   OFF          DEPENDS Rasqal DOC "Use the Redland RDF library")
mitkFunctionAddExternalProject(NAME SOFA      OFF          DEPENDS GLUT Boost DOC "Use Simulation Open Framework Architecture")

if(MITK_USE_QT)
  mitkFunctionAddExternalProject(NAME Qwt     ON  ADVANCED DEPENDS QT)
endif()

#-----------------------------------------------------------------------------
# Build configurations
#-----------------------------------------------------------------------------

set(_buildConfigs "Custom")

file(GLOB _buildConfigFiles CMake/BuildConfigurations/*.cmake)

foreach(_buildConfigFile ${_buildConfigFiles})
  get_filename_component(_buildConfigFile ${_buildConfigFile} NAME_WE)
  list(APPEND _buildConfigs ${_buildConfigFile})
endforeach()

set(MITK_BUILD_CONFIGURATION "Custom" CACHE STRING "Use pre-defined MITK configurations")
mark_as_advanced(MITK_BUILD_CONFIGURATION)
set_property(CACHE MITK_BUILD_CONFIGURATION PROPERTY STRINGS ${_buildConfigs})

mitkFunctionEnableBuildConfiguration()

mitkFunctionCreateWhitelistPaths(MITK)
mitkFunctionFindWhitelists(MITK)

# -----------------------------------------
# Custom dependency logic

if(MITK_USE_Boost)
  option(MITK_USE_SYSTEM_Boost "Use the system Boost" OFF)
  set(MITK_USE_Boost_LIBRARIES "" CACHE STRING "A semi-colon separated list of required Boost libraries")
endif()

if(MITK_USE_SOFA)
  # SOFA requires at least CMake 2.8.8
  set(SOFA_CMAKE_VERSION 2.8.8)
  if(${CMAKE_VERSION} VERSION_LESS ${SOFA_CMAKE_VERSION})
    set(MITK_USE_SOFA OFF CACHE BOOL "" FORCE)
    message(WARNING "Switched off MITK_USE_SOFA\n  Minimum required CMake version: ${SOFA_CMAKE_VERSION}\n  Installed CMake version: ${CMAKE_VERSION}")
  endif()
  # SOFA/ITK combination requires at least MSVC 2010
  if(MSVC_VERSION AND MSVC_VERSION LESS 1600)
    set(MITK_USE_SOFA OFF CACHE BOOL "" FORCE)
    message(WARNING "Switched off MITK_USE_SOFA\n  MSVC versions less than 2010 are not supported.")
  endif()
  # SOFA requires boost library
  if(MITK_USE_SOFA AND NOT MITK_USE_Boost)
    message("> Forcing MITK_USE_Boost to ON because of MITK_USE_SOFA")
    set(MITK_USE_Boost ON CACHE BOOL "" FORCE)
  endif()
  # SOFA requires boost system library
  list(FIND MITK_USE_Boost_LIBRARIES system _result)
  if(_result LESS 0)
    message("> Adding 'system' to MITK_USE_Boost_LIBRARIES.")
    list(APPEND MITK_USE_Boost_LIBRARIES system)
  endif()
  # SOFA requires boost thread library
  list(FIND MITK_USE_Boost_LIBRARIES thread _result)
  if(_result LESS 0)
    message("> Adding 'thread' to MITK_USE_Boost_LIBRARIES.")
    list(APPEND MITK_USE_Boost_LIBRARIES thread)
  endif()
  # Simulation plugin requires boost chrono library
  list(FIND MITK_USE_Boost_LIBRARIES chrono _result)
  if(_result LESS 0)
    message("> Adding 'chrono' to MITK_USE_Boost_LIBRARIES.")
    list(APPEND MITK_USE_Boost_LIBRARIES chrono)
  endif()
  set(MITK_USE_Boost_LIBRARIES ${MITK_USE_Boost_LIBRARIES} CACHE STRING "" FORCE)
  # Allow setting external SOFA plugins directory and SOFA plugins
  set(MITK_USE_SOFA_PLUGINS_DIR ${MITK_USE_SOFA_PLUGINS_DIR} CACHE PATH "External SOFA plugins directory" FORCE)
  set(MITK_USE_SOFA_PLUGINS ${MITK_USE_SOFA_PLUGINS} CACHE PATH "List of semicolon-separated plugin names" FORCE)
endif()

if(MITK_USE_Python AND NOT MITK_USE_SYSTEM_PYTHON)
  set(MITK_USE_ZLIB ON)
  if(NOT MITK_USE_Numpy)
    message("> Forcing MITK_USE_Numpy to ON because of MITK_USE_Python")
    set(MITK_USE_Numpy ON CACHE BOOL "Use Numpy" FORCE)
  endif()
  if(NOT MITK_USE_SimpleITK)
    message("> Forcing MITK_USE_SimpleITK to ON because of MITK_USE_Python")
    set(MITK_USE_Numpy ON CACHE BOOL "Use Numpy" FORCE)
  endif()

  if(APPLE)
    message(WARNING "Python wrapping is unsuported on mac OSX!")
    set(MITK_USE_Python OFF CACHE BOOL "Use Python wrapping in MITK" FORCE)
  else()
    option(MITK_USE_SYSTEM_PYTHON "Use the system python runtime" OFF)
    if(MITK_USE_SYSTEM_PYTHON)
      find_package(PythonLibs REQUIRED)
      find_package(PythonInterp REQUIRED)
    endif()
  endif()
endif()

if(BUILD_TESTING AND NOT MITK_USE_CppUnit)
  message("> Forcing MITK_USE_CppUnit to ON because BUILD_TESTING=ON")
  set(MITK_USE_CppUnit ON CACHE BOOL "Use CppUnit for unit tests" FORCE)
endif()

# -----------------------------------------
# Other MITK_USE_* options not related to
# external projects build via the
# MITK superbuild

env_option(MITK_USE_BLUEBERRY "Build the BlueBerry platform" ON)
env_option(MITK_USE_OpenCL "Use OpenCL GPU-Computing library" OFF)

if(MITK_USE_BLUEBERRY)
  option(MITK_BUILD_ALL_PLUGINS "Build all MITK plugins" OFF)
  mark_as_advanced(MITK_BUILD_ALL_PLUGINS)

  if(NOT MITK_USE_CTK)
    message("> Forcing MITK_USE_CTK to ON because of MITK_USE_BLUEBERRY")
    set(MITK_USE_CTK ON CACHE BOOL "Use CTK in MITK" FORCE)
  endif()
endif()

#-----------------------------------------------------------------------------
# Pixel type multiplexing
#-----------------------------------------------------------------------------

# Customize the default pixel types for multiplex macros

set(MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES
    "int, unsigned int, short, unsigned short, char, unsigned char"
    CACHE STRING "List of integral pixel types used in AccessByItk and InstantiateAccessFunction macros")

set(MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES
    "double, float"
    CACHE STRING "List of floating pixel types used in AccessByItk and InstantiateAccessFunction macros")

set(MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES
    "itk::RGBPixel<unsigned char>, itk::RGBAPixel<unsigned char>"
    CACHE STRING "List of composite pixel types used in AccessByItk and InstantiateAccessFunction macros")

set(MITK_ACCESSBYITK_DIMENSIONS
    "2,3"
    CACHE STRING "List of dimensions used in AccessByItk and InstantiateAccessFunction macros")

mark_as_advanced(MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES
                 MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES
                 MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES
                 MITK_ACCESSBYITK_DIMENSIONS
                )

# consistency checks

if(NOT MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES)
  set(MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES
      "int, unsigned int, short, unsigned short, char, unsigned char"
      CACHE STRING "List of integral pixel types used in AccessByItk and InstantiateAccessFunction macros" FORCE)
endif()

if(NOT MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES)
  set(MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES
      "double, float"
      CACHE STRING "List of floating pixel types used in AccessByItk and InstantiateAccessFunction macros" FORCE)
endif()

if(NOT MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES)
  set(MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES
    "itk::RGBPixel<unsigned char>, itk::RGBAPixel<unsigned char>"
    CACHE STRING "List of composite pixel types used in AccessByItk and InstantiateAccessFunction macros" FORCE)
endif()

if(NOT MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES)
  string(REPLACE "," ";" _integral_types ${MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES})
  string(REPLACE "," ";" _floating_types ${MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES})
  foreach(_scalar_type ${_integral_types} ${_floating_types})
    set(MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES
        "${MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES}itk::VariableLengthVector<${_scalar_type}>,")
  endforeach()
  string(LENGTH "${MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES}" _length)
  math(EXPR _length "${_length} - 1")
  string(SUBSTRING "${MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES}" 0 ${_length} MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES)
  set(MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES ${MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES}
      CACHE STRING "List of vector pixel types used in AccessByItk and InstantiateAccessFunction macros for itk::VectorImage types" FORCE)
endif()

if(NOT MITK_ACCESSBYITK_DIMENSIONS)
  set(MITK_ACCESSBYITK_DIMENSIONS
      "2,3"
      CACHE STRING "List of dimensions used in AccessByItk and InstantiateAccessFunction macros")
endif()

#-----------------------------------------------------------------------------
# Project.xml
#-----------------------------------------------------------------------------

# A list of topologically ordered targets
set(CTEST_PROJECT_SUBPROJECTS)
if(MITK_USE_BLUEBERRY)
  list(APPEND CTEST_PROJECT_SUBPROJECTS BlueBerry)
endif()

list(APPEND CTEST_PROJECT_SUBPROJECTS
  MITK-Core
  MITK-CoreUI
  MITK-IGT
  MITK-ToF
  MITK-DTI
  MITK-Registration
  MITK-Modules # all modules not contained in a specific subproject
  MITK-Plugins # all plugins not contained in a specific subproject
  MITK-Examples
  Unlabeled # special "subproject" catching all unlabeled targets and tests
  )

# Configure CTestConfigSubProject.cmake that could be used by CTest scripts
configure_file(${MITK_SOURCE_DIR}/CTestConfigSubProject.cmake.in
               ${MITK_BINARY_DIR}/CTestConfigSubProject.cmake)

if(CTEST_PROJECT_ADDITIONAL_TARGETS)
  # those targets will be executed at the end of the ctest driver script
  # and they also get their own subproject label
  set(subproject_list "${CTEST_PROJECT_SUBPROJECTS};${CTEST_PROJECT_ADDITIONAL_TARGETS}")
else()
  set(subproject_list "${CTEST_PROJECT_SUBPROJECTS}")
endif()

# Generate Project.xml file expected by the CTest driver script
mitkFunctionGenerateProjectXml(${MITK_BINARY_DIR} MITK "${subproject_list}" ${MITK_USE_SUPERBUILD})

#-----------------------------------------------------------------------------
# Superbuild script
#-----------------------------------------------------------------------------

if(MITK_USE_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")

  # Print configuration summary
  message("\n\n")
  feature_summary(
    DESCRIPTION "------- FEATURE SUMMARY FOR ${PROJECT_NAME} -------"
    WHAT ALL)
  return()
endif()

#*****************************************************************************
#****************************  END OF SUPERBUILD  ****************************
#*****************************************************************************

#-----------------------------------------------------------------------------
# CMake function(s) and macro(s)
#-----------------------------------------------------------------------------

include(WriteBasicConfigVersionFile)
include(CheckCXXSourceCompiles)
include(GenerateExportHeader)
include(mitkFunctionOrganizeSources)
include(mitkFunctionGetVersion)
include(mitkFunctionGetVersionDescription)
include(mitkFunctionCreateWindowsBatchScript)
include(mitkFunctionInstallProvisioningFiles)
include(mitkFunctionInstallAutoLoadModules)
include(mitkFunctionGetLibrarySearchPaths)
include(mitkFunctionCompileSnippets)
include(mitkFunctionUseModules)
include(mitkFunctionCheckModuleDependencies)
include(mitkFunctionCreateModule)
include(mitkMacroCreateExecutable)
include(mitkMacroCreateModuleTests)
include(mitkFunctionAddCustomModuleTest)
include(mitkMacroMultiplexPicType)
include(mitkMacroInstall)
include(mitkMacroInstallHelperApp)
include(mitkMacroInstallTargets)
include(mitkMacroGenerateToolsLibrary)
include(mitkMacroGetLinuxDistribution)
include(mitkMacroGetPMDPlatformString)
include(mitkMacroConfigureItkPixelTypes)

#-----------------------------------------------------------------------------
# Global CMake variables
#-----------------------------------------------------------------------------

# Required and enabled C++11 features for all MITK code.
# These are added as PUBLIC compile features to all MITK modules.
set(MITK_CXX_FEATURES cxx_auto_type cxx_nullptr cxx_override)

if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
  # We can't do this yet because the CTK Plugin Framework
  # cannot cope with a postfix yet.
  #set(CMAKE_DEBUG_POSTFIX d)
endif()

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

set(_default_LIBRARY_output_dir lib)
set(_default_RUNTIME_output_dir bin)
set(_default_ARCHIVE_output_dir lib)

foreach(type LIBRARY RUNTIME ARCHIVE)
  # Make sure the directory exists
  if(MITK_CMAKE_${type}_OUTPUT_DIRECTORY
     AND NOT EXISTS ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY})
    message("Creating directory MITK_CMAKE_${type}_OUTPUT_DIRECTORY: ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY}")
    file(MAKE_DIRECTORY "${MITK_CMAKE_${type}_OUTPUT_DIRECTORY}")
  endif()

  if(MITK_CMAKE_${type}_OUTPUT_DIRECTORY)
    set(CMAKE_${type}_OUTPUT_DIRECTORY ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY})
  else()
    set(CMAKE_${type}_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${_default_${type}_output_dir})
    set(MITK_CMAKE_${type}_OUTPUT_DIRECTORY ${CMAKE_${type}_OUTPUT_DIRECTORY})
  endif()

  set(CMAKE_${type}_OUTPUT_DIRECTORY ${CMAKE_${type}_OUTPUT_DIRECTORY} CACHE INTERNAL "Output directory for ${type} files.")
  mark_as_advanced(CMAKE_${type}_OUTPUT_DIRECTORY)
endforeach()

#-----------------------------------------------------------------------------
# Set MITK specific options and variables (NOT available during superbuild)
#-----------------------------------------------------------------------------

# ASK THE USER TO SHOW THE CONSOLE WINDOW FOR CoreApp and mitkWorkbench
option(MITK_SHOW_CONSOLE_WINDOW "Use this to enable or disable the console window when starting MITK GUI Applications" ON)
mark_as_advanced(MITK_SHOW_CONSOLE_WINDOW)

# TODO: check if necessary
option(USE_ITKZLIB "Use the ITK zlib for pic compression." ON)
mark_as_advanced(USE_ITKZLIB)

if(NOT MITK_FAST_TESTING)
  if(DEFINED MITK_CTEST_SCRIPT_MODE
      AND (MITK_CTEST_SCRIPT_MODE STREQUAL "continuous" OR MITK_CTEST_SCRIPT_MODE STREQUAL "experimental") )
    set(MITK_FAST_TESTING 1)
  endif()
endif()

# MITK_VERSION
set(MITK_VERSION_STRING "${MITK_VERSION_MAJOR}.${MITK_VERSION_MINOR}.${MITK_VERSION_PATCH}")
if(MITK_VERSION_PATCH STREQUAL "99")
  set(MITK_VERSION_STRING "${MITK_VERSION_STRING}-${MITK_REVISION_SHORTID}")
endif()

# Needed early on for redirecting the BlueBerry documentation output dir
set(MITK_DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/Documentation/Doxygen CACHE PATH
    "Output directory for doxygen generated documentation." )

if(NOT UNIX AND NOT MINGW)
  set(MITK_WIN32_FORCE_STATIC "STATIC" CACHE INTERNAL "Use this variable to always build static libraries on non-unix platforms")
endif()

if(MITK_BUILD_ALL_PLUGINS)
  set(MITK_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
endif()

# Configure pixel types used for ITK image access multiplexing
mitkMacroConfigureItkPixelTypes()

# Configure module naming conventions
set(MITK_MODULE_NAME_REGEX_MATCH "^[A-Z].*$")
set(MITK_MODULE_NAME_REGEX_NOT_MATCH "^[Mm][Ii][Tt][Kk].*$")
set(MITK_MODULE_NAME_PREFIX "Mitk")
set(MITK_MODULE_NAME_DEFAULTS_TO_DIRECTORY_NAME 1)

#-----------------------------------------------------------------------------
# Get MITK version info
#-----------------------------------------------------------------------------

mitkFunctionGetVersion(${MITK_SOURCE_DIR} MITK)
mitkFunctionGetVersionDescription(${MITK_SOURCE_DIR} MITK)

#-----------------------------------------------------------------------------
# Installation preparation
#
# These should be set before any MITK install macros are used
#-----------------------------------------------------------------------------

# on Mac OSX all BlueBerry plugins get copied into every
# application bundle (.app directory) specified here
if(MITK_USE_BLUEBERRY AND APPLE)

  include("${CMAKE_CURRENT_SOURCE_DIR}/Applications/AppList.cmake")

  foreach(mitk_app ${MITK_APPS})
    # extract option_name
    string(REPLACE "^^" "\\;" target_info ${mitk_app})
    set(target_info_list ${target_info})
    list(GET target_info_list 1 option_name)
    list(GET target_info_list 0 app_name)
    # check if the application is enabled
    if(${option_name} OR MITK_BUILD_ALL_APPS)
      set(MACOSX_BUNDLE_NAMES ${MACOSX_BUNDLE_NAMES} Mitk${app_name})
    endif()
  endforeach()

endif()

#-----------------------------------------------------------------------------
# Set coverage Flags
#-----------------------------------------------------------------------------

if(WITH_COVERAGE)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(coverage_flags "-g -fprofile-arcs -ftest-coverage -O0 -DNDEBUG")
    set(COVERAGE_CXX_FLAGS ${coverage_flags})
    set(COVERAGE_C_FLAGS ${coverage_flags})
  endif()
endif()

#-----------------------------------------------------------------------------
# MITK C/CXX Flags
#-----------------------------------------------------------------------------

set(MITK_C_FLAGS "${COVERAGE_C_FLAGS}")
set(MITK_C_FLAGS_DEBUG )
set(MITK_C_FLAGS_RELEASE )
set(MITK_CXX_FLAGS "${COVERAGE_CXX_FLAGS} ${MITK_CXX11_FLAG}")
set(MITK_CXX_FLAGS_DEBUG )
set(MITK_CXX_FLAGS_RELEASE )

set(MITK_EXE_LINKER_FLAGS )
set(MITK_SHARED_LINKER_FLAGS )

if(WIN32)
  set(MITK_CXX_FLAGS "${MITK_CXX_FLAGS} -D_WIN32_WINNT=0x0501 -DPOCO_NO_UNWINDOWS -DWIN32_LEAN_AND_MEAN -DNOMINMAX")
  mitkFunctionCheckCompilerFlags("/wd4005" MITK_CXX_FLAGS) # warning C4005: macro redefinition
  mitkFunctionCheckCompilerFlags("/wd4231" MITK_CXX_FLAGS) # warning C4231: nonstandard extension used : 'extern' before template explicit instantiation
  # the following line should be removed after fixing bug 17637
  mitkFunctionCheckCompilerFlags("/wd4316" MITK_CXX_FLAGS) # warning C4316: object alignment on heap
endif()

if(NOT MSVC_VERSION)
  foreach(_flag
    -Wall
    -Wextra
    -Wpointer-arith
    -Winvalid-pch
    -Wcast-align
    -Wwrite-strings
    -Wno-error=gnu
    -Wno-error=unknown-pragmas
    # The strict-overflow warning is generated by ITK template code
    -Wno-error=strict-overflow
    -Woverloaded-virtual
    -Wstrict-null-sentinel
    #-Wold-style-cast
    #-Wsign-promo

    # the following two lines should be removed after ITK-3097 has
    # been resolved, see also MITK bug 15279
    -Wno-unused-local-typedefs
    -Wno-array-bounds

    -fdiagnostics-show-option
    )
    mitkFunctionCheckCAndCXXCompilerFlags(${_flag} MITK_C_FLAGS MITK_CXX_FLAGS)
  endforeach()
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
  mitkFunctionCheckCompilerFlags("-Wl,--no-undefined" MITK_SHARED_LINKER_FLAGS)
  mitkFunctionCheckCompilerFlags("-Wl,--as-needed" MITK_SHARED_LINKER_FLAGS)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  # With older version of gcc supporting the flag -fstack-protector-all, an extra dependency to libssp.so
  # is introduced. If gcc is smaller than 4.4.0 and the build type is Release let's not include the flag.
  # Doing so should allow to build package made for distribution using older linux distro.
  if(${GCC_VERSION} VERSION_GREATER "4.4.0" OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ${GCC_VERSION} VERSION_LESS "4.4.0"))
    mitkFunctionCheckCAndCXXCompilerFlags("-fstack-protector-all" MITK_C_FLAGS MITK_CXX_FLAGS)
  endif()
  if(MINGW)
    # suppress warnings about auto imported symbols
    set(MITK_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import ${MITK_SHARED_LINKER_FLAGS}")
  endif()

  set(MITK_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 ${MITK_CXX_FLAGS_RELEASE}")
endif()

set(MITK_MODULE_LINKER_FLAGS ${MITK_SHARED_LINKER_FLAGS})
set(MITK_EXE_LINKER_FLAGS ${MITK_SHARED_LINKER_FLAGS})

#-----------------------------------------------------------------------------
# MITK Packages
#-----------------------------------------------------------------------------

set(MITK_MODULES_PACKAGE_DEPENDS_DIR ${MITK_SOURCE_DIR}/CMake/PackageDepends)
set(MODULES_PACKAGE_DEPENDS_DIRS ${MITK_MODULES_PACKAGE_DEPENDS_DIR})

if(NOT MITK_USE_SYSTEM_Boost)
  set(Boost_NO_SYSTEM_PATHS 1)
endif()

set(Boost_USE_MULTITHREADED 1)
set(Boost_USE_STATIC_LIBS 0)
set(Boost_USE_STATIC_RUNTIME 0)

# We need this later for a DCMTK workaround
set(_dcmtk_dir_orig ${DCMTK_DIR})

# This property is populated at the top half of this file
get_property(MITK_EXTERNAL_PROJECTS GLOBAL PROPERTY MITK_EXTERNAL_PROJECTS)
foreach(ep ${MITK_EXTERNAL_PROJECTS})
  get_property(_package GLOBAL PROPERTY MITK_${ep}_PACKAGE)
  get_property(_components GLOBAL PROPERTY MITK_${ep}_COMPONENTS)
  if(MITK_USE_${ep} AND _package)
    if(_components)
      find_package(${_package} COMPONENTS ${_components} REQUIRED CONFIG)
    else()
      # Prefer config mode first because it finds external
      # <proj>Config.cmake files pointed at by <proj>_DIR variables.
      # Otherwise, existing Find<proj>.cmake files could fail.
      # (e.g. in the case of GLEW and the FindGLEW.cmake file shipped
      # with CMake).
      find_package(${_package} QUIET CONFIG)
      string(TOUPPER "${_package}" _package_uc)
      if(NOT (${_package}_FOUND OR ${_package_uc}_FOUND))
        find_package(${_package} REQUIRED)
      endif()
    endif()
  endif()
endforeach()

# Ensure that the MITK CMake module path comes first
set(CMAKE_MODULE_PATH
  ${MITK_CMAKE_DIR}
  ${CMAKE_MODULE_PATH}
  )

if(MITK_USE_DCMTK)
  # Due to the preferred CONFIG mode in find_package calls above,
  # the DCMTKConfig.cmake file is read, which does not provide useful
  # package information. We explictly need MODULE mode to find DCMTK.
  if(${_dcmtk_dir_orig} MATCHES "${MITK_EXTERNAL_PROJECT_PREFIX}.*")
    # Help our FindDCMTK.cmake script find our super-build DCMTK
    set(DCMTK_DIR ${MITK_EXTERNAL_PROJECT_PREFIX})
  else()
    # Use the original value
    set(DCMTK_DIR ${_dcmtk_dir_orig})
  endif()
  find_package(DCMTK REQUIRED MODULE)
endif()

if(MITK_USE_Python)
  find_package(PythonLibs REQUIRED)
  find_package(PythonInterp REQUIRED)
endif()

if(MITK_USE_SOFA)
  # The SOFAConfig.cmake file does not provide exported targets or
  # libraries with absolute paths, hence we need to make the link
  # directories globally available until the SOFAConfig.cmake file
  # supports a proper mechanism for handling targets.
  # The same code is needed in MITKConfig.cmake.
  link_directories(${SOFA_LIBRARY_DIRS})
endif()

if(MITK_USE_Boost)
  # Same as SOFA above
  link_directories(${Boost_LIBRARY_DIRS})
endif()

if(MITK_USE_OpenIGTLink)
  # Same as SOFA above
  link_directories(${OpenIGTLink_LIBRARY_DIRS})
endif()

if(MITK_USE_SimpleITK)
  link_directories(${SimpleITK_LIBRARY_DIRS})
endif()

# Qt support
if(MITK_USE_QT)
  if(DESIRED_QT_VERSION MATCHES 4)
    find_package(Qt4 ${MITK_QT4_MINIMUM_VERSION} REQUIRED)
  elseif(DESIRED_QT_VERSION MATCHES 5)
    find_package(Qt5Core ${MITK_QT5_MINIMUM_VERSION} REQUIRED) # at least Core required
    get_target_property(_qmake_exec Qt5::qmake LOCATION)
    execute_process(COMMAND ${_qmake_exec} -query QT_INSTALL_BINS
      RESULT_VARIABLE _result
      OUTPUT_VARIABLE QT_BINARY_DIR
      ERROR_VARIABLE _error
    )
    string(STRIP "${QT_BINARY_DIR}" QT_BINARY_DIR)
    if(_result OR NOT EXISTS "${QT_BINARY_DIR}")
      message(FATAL_ERROR "Could not determine Qt binary directory: ${_result} ${QT_BINARY_DIR} ${_error}")
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
  enable_testing()
  include(CTest)
  mark_as_advanced(TCL_TCLSH DART_ROOT)

  option(MITK_ENABLE_RENDERING_TESTING OFF "Enable the MITK rendering tests. Requires x-server in Linux.")
  #Rendering testing does not work for Linux nightlies, thus it is disabled per default
  #and activated for Mac and Windows.
  if(WIN32 OR APPLE)
    set(MITK_ENABLE_RENDERING_TESTING ON)
  endif()
  mark_as_advanced( MITK_ENABLE_RENDERING_TESTING )

  # Setup file for setting custom ctest vars
  configure_file(
    CMake/CTestCustom.cmake.in
    ${MITK_BINARY_DIR}/CTestCustom.cmake
    @ONLY
    )

  # Configuration for the CMake-generated test driver
  set(CMAKE_TESTDRIVER_EXTRA_INCLUDES "#include <stdexcept>")
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "
    try
      {")
  set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "    }
      catch( std::exception & excp )
        {
        fprintf(stderr,\"%s\\n\",excp.what());
        return EXIT_FAILURE;
        }
      catch( ... )
        {
        printf(\"Exception caught in the test driver\\n\");
        return EXIT_FAILURE;
        }
      ")

  set(MITK_TEST_OUTPUT_DIR "${MITK_BINARY_DIR}/test_output")
  if(NOT EXISTS ${MITK_TEST_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${MITK_TEST_OUTPUT_DIR})
  endif()

  # Test the external project template
  if(MITK_USE_BLUEBERRY)
    #include(mitkTestProjectTemplate)
  endif()

  # Test the package target
  include(mitkPackageTest)
endif()

configure_file(mitkTestingConfig.h.in ${MITK_BINARY_DIR}/mitkTestingConfig.h)

#-----------------------------------------------------------------------------
# MITK_SUPERBUILD_BINARY_DIR
#-----------------------------------------------------------------------------

# If MITK_SUPERBUILD_BINARY_DIR isn't defined, it means MITK is *NOT* build using Superbuild.
# In that specific case, MITK_SUPERBUILD_BINARY_DIR should default to MITK_BINARY_DIR
if(NOT DEFINED MITK_SUPERBUILD_BINARY_DIR)
  set(MITK_SUPERBUILD_BINARY_DIR ${MITK_BINARY_DIR})
endif()

#-----------------------------------------------------------------------------
# Set C/CXX and linker flags for MITK code
#-----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MITK_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MITK_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MITK_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MITK_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${MITK_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${MITK_C_FLAGS_RELEASE}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MITK_EXE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${MITK_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${MITK_MODULE_LINKER_FLAGS}")

#-----------------------------------------------------------------------------
# Compile Utilities and set-up MITK variables
#-----------------------------------------------------------------------------

add_subdirectory(Utilities)

if(MITK_USE_BLUEBERRY)

  # We need to hack a little bit because MITK applications may need
  # to enable certain BlueBerry plug-ins. However, these plug-ins
  # are validated separately from the MITK plug-ins and know nothing
  # about potential MITK plug-in dependencies of the applications. Hence
  # we cannot pass the MITK application list to the BlueBerry
  # ctkMacroSetupPlugins call but need to extract the BlueBerry dependencies
  # from the applications and set them explicitly.

  include("${CMAKE_CURRENT_SOURCE_DIR}/Applications/AppList.cmake")

  foreach(mitk_app ${MITK_APPS})
    # extract target_dir and option_name
    string(REPLACE "^^" "\\;" target_info ${mitk_app})
    set(target_info_list ${target_info})
    list(GET target_info_list 0 target_dir)
    list(GET target_info_list 1 option_name)
    # check if the application is enabled and if target_libraries.cmake exists
    if((${option_name} OR MITK_BUILD_ALL_APPS) AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Applications/${target_dir}/target_libraries.cmake")
      include("${CMAKE_CURRENT_SOURCE_DIR}/Applications/${target_dir}/target_libraries.cmake")
      foreach(_target_dep ${target_libraries})
        if(_target_dep MATCHES org_blueberry_)
          string(REPLACE _ . _app_bb_dep ${_target_dep})
          # explicitly set the build option for the BlueBerry plug-in
          set(BLUEBERRY_BUILD_${_app_bb_dep} ON CACHE BOOL "Build the ${_app_bb_dep} plug-in")
        endif()
      endforeach()
    endif()
  endforeach()

  set(mbilog_DIR "${mbilog_BINARY_DIR}")

  if(MITK_BUILD_ALL_PLUGINS)
    set(BLUEBERRY_BUILD_ALL_PLUGINS ON)
  endif()

  set(BLUEBERRY_CXX_FLAGS "${MITK_CXX_FLAGS}")
  set(BLUEBERRY_CXX_FLAGS_RELEASE "${MITK_CXX_FLAGS_RELEASE}")
  set(BLUEBERRY_C_FLAGS "${MITK_C_FLAGS}")

  set(BLUEBERRY_XPDOC_OUTPUT_DIR ${MITK_DOXYGEN_OUTPUT_DIR}/html/extension-points/html/)

  add_subdirectory(BlueBerry)

  set(BlueBerry_DIR ${CMAKE_CURRENT_BINARY_DIR}/BlueBerry
      CACHE PATH "The directory containing a CMake configuration file for BlueBerry" FORCE)

  include(mitkMacroCreateCTKPlugin)

endif()

#-----------------------------------------------------------------------------
# Add custom targets representing CDash subprojects
#-----------------------------------------------------------------------------

foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
  if(NOT TARGET ${subproject} AND NOT subproject MATCHES "Unlabeled")
    add_custom_target(${subproject})
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Add subdirectories
#-----------------------------------------------------------------------------

add_subdirectory(Modules)

if(MITK_USE_BLUEBERRY)

  find_package(BlueBerry REQUIRED)

  set(MITK_DEFAULT_SUBPROJECTS MITK-Plugins)

  # Plug-in testing (needs some work to be enabled again)
  if(BUILD_TESTING)
    include(berryTestingHelpers)

    set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp")
    if(TARGET CoreApp)
      get_target_property(_is_macosx_bundle CoreApp MACOSX_BUNDLE)
      if(APPLE AND _is_macosx_bundle)
        set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp.app/Contents/MacOS/CoreApp")
      endif()
    endif()
    set(BLUEBERRY_TEST_APP_ID "org.mitk.qt.coreapplication")
  endif()

  include("${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PluginList.cmake")
  mitkFunctionWhitelistPlugins(MITK MITK_EXT_PLUGINS)

  set(mitk_plugins_fullpath "")
  foreach(mitk_plugin ${MITK_EXT_PLUGINS})
    list(APPEND mitk_plugins_fullpath Plugins/${mitk_plugin})
  endforeach()

  if(EXISTS ${MITK_PRIVATE_MODULES}/PluginList.cmake)
    include(${MITK_PRIVATE_MODULES}/PluginList.cmake)

    foreach(mitk_plugin ${MITK_PRIVATE_PLUGINS})
      list(APPEND mitk_plugins_fullpath ${MITK_PRIVATE_MODULES}/${mitk_plugin})
    endforeach()
  endif()

  if(MITK_BUILD_EXAMPLES)
    include("${CMAKE_CURRENT_SOURCE_DIR}/Examples/Plugins/PluginList.cmake")
    set(mitk_example_plugins_fullpath )
    foreach(mitk_example_plugin ${MITK_EXAMPLE_PLUGINS})
      list(APPEND mitk_example_plugins_fullpath Examples/Plugins/${mitk_example_plugin})
      list(APPEND mitk_plugins_fullpath Examples/Plugins/${mitk_example_plugin})
    endforeach()
  endif()

  # Specify which plug-ins belong to this project
  macro(GetMyTargetLibraries all_target_libraries varname)
    set(re_ctkplugin_mitk "^org_mitk_[a-zA-Z0-9_]+$")
    set(re_ctkplugin_bb "^org_blueberry_[a-zA-Z0-9_]+$")
    set(_tmp_list)
    list(APPEND _tmp_list ${all_target_libraries})
    ctkMacroListFilter(_tmp_list re_ctkplugin_mitk re_ctkplugin_bb OUTPUT_VARIABLE ${varname})
  endmacro()

  # Get infos about application directories and build options
  include("${CMAKE_CURRENT_SOURCE_DIR}/Applications/AppList.cmake")
  set(mitk_apps_fullpath )
  foreach(mitk_app ${MITK_APPS})
    string(FIND ${mitk_app} "MITK_BUILD_APP_" _index)
    string(SUBSTRING ${mitk_app} ${_index} -1 _var)
    if(${_var})
      list(APPEND mitk_apps_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/Applications/${mitk_app}")
    endif()
  endforeach()

  if (mitk_plugins_fullpath)
    ctkMacroSetupPlugins(${mitk_plugins_fullpath}
                         BUILD_OPTION_PREFIX MITK_BUILD_
                         APPS ${mitk_apps_fullpath}
                         BUILD_ALL ${MITK_BUILD_ALL_PLUGINS}
                         COMPACT_OPTIONS)
  endif()

  set(MITK_PLUGIN_USE_FILE "${MITK_BINARY_DIR}/MitkPluginUseFile.cmake")
  if(${PROJECT_NAME}_PLUGIN_LIBRARIES)
    ctkFunctionGeneratePluginUseFile(${MITK_PLUGIN_USE_FILE})
  else()
    file(REMOVE ${MITK_PLUGIN_USE_FILE})
    set(MITK_PLUGIN_USE_FILE )
  endif()
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(Documentation)

#-----------------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------------


# set MITK cpack variables
# These are the default variables, which can be overwritten ( see below )
include(mitkSetupCPack)

set(use_default_config ON)

# MITK_APPS is set in Applications/AppList.cmake (included somewhere above
# if MITK_USE_BLUEBERRY is set to ON).
if(MITK_APPS)

  set(activated_apps_no 0)
  list(LENGTH MITK_APPS app_count)

  # Check how many apps have been enabled
  # If more than one app has been activated, the we use the
  # default CPack configuration. Otherwise that apps configuration
  # will be used, if present.
  foreach(mitk_app ${MITK_APPS})
    # extract option_name
    string(REPLACE "^^" "\\;" target_info ${mitk_app})
    set(target_info_list ${target_info})
    list(GET target_info_list 1 option_name)
    # check if the application is enabled
    if(${option_name} OR MITK_BUILD_ALL_APPS)
      MATH(EXPR activated_apps_no "${activated_apps_no} + 1")
    endif()
  endforeach()

  if(app_count EQUAL 1 AND (activated_apps_no EQUAL 1 OR MITK_BUILD_ALL_APPS))
    # Corner case if there is only one app in total
    set(use_project_cpack ON)
  elseif(activated_apps_no EQUAL 1 AND NOT MITK_BUILD_ALL_APPS)
    # Only one app is enabled (no "build all" flag set)
    set(use_project_cpack ON)
  else()
    # Less or more then one app is enabled
    set(use_project_cpack OFF)
  endif()

  foreach(mitk_app ${MITK_APPS})
    # extract target_dir and option_name
    string(REPLACE "^^" "\\;" target_info ${mitk_app})
    set(target_info_list ${target_info})
    list(GET target_info_list 0 target_dir)
    list(GET target_info_list 1 option_name)
    # check if the application is enabled
    if(${option_name} OR MITK_BUILD_ALL_APPS)
      # check whether application specific configuration files will be used
      if(use_project_cpack)
        # use files if they exist
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Applications/${target_dir}/CPackOptions.cmake")
          include("${CMAKE_CURRENT_SOURCE_DIR}/Applications/${target_dir}/CPackOptions.cmake")
        endif()

        if(EXISTS "${PROJECT_SOURCE_DIR}/Applications/${target_dir}/CPackConfig.cmake.in")
          set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/Applications/${target_dir}/CPackConfig.cmake")
          configure_file(${PROJECT_SOURCE_DIR}/Applications/${target_dir}/CPackConfig.cmake.in
                         ${CPACK_PROJECT_CONFIG_FILE} @ONLY)
          set(use_default_config OFF)
        endif()
      endif()
    # add link to the list
    list(APPEND CPACK_CREATE_DESKTOP_LINKS "${target_dir}")
    endif()
  endforeach()

endif()

# if no application specific configuration file was used, use default
if(use_default_config)
  configure_file(${MITK_SOURCE_DIR}/MITKCPackOptions.cmake.in
                 ${MITK_BINARY_DIR}/MITKCPackOptions.cmake @ONLY)
  set(CPACK_PROJECT_CONFIG_FILE "${MITK_BINARY_DIR}/MITKCPackOptions.cmake")
endif()

# include CPack model once all variables are set
include(CPack)

# Additional installation rules
include(mitkInstallRules)

#-----------------------------------------------------------------------------
# Last configuration steps
#-----------------------------------------------------------------------------

# ---------------- Export targets -----------------

set(MITK_EXPORTS_FILE "${MITK_BINARY_DIR}/MitkExports.cmake")
file(REMOVE ${MITK_EXPORTS_FILE})

set(targets_to_export)
get_property(module_targets GLOBAL PROPERTY MITK_MODULE_TARGETS)
if(module_targets)
  list(APPEND targets_to_export ${module_targets})
endif()

if(MITK_USE_BLUEBERRY)
  if(MITK_PLUGIN_LIBRARIES)
    list(APPEND targets_to_export ${MITK_PLUGIN_LIBRARIES})
  endif()
endif()

export(TARGETS ${targets_to_export} APPEND
       FILE ${MITK_EXPORTS_FILE})

set(MITK_EXPORTED_TARGET_PROPERTIES )
foreach(target_to_export ${targets_to_export})
  get_target_property(autoload_targets ${target_to_export} MITK_AUTOLOAD_TARGETS)
  if(autoload_targets)
    set(MITK_EXPORTED_TARGET_PROPERTIES "${MITK_EXPORTED_TARGET_PROPERTIES}
set_target_properties(${target_to_export} PROPERTIES MITK_AUTOLOAD_TARGETS \"${autoload_targets}\")")
  endif()
  get_target_property(autoload_dir ${target_to_export} MITK_AUTOLOAD_DIRECTORY)
  if(autoload_dir)
    set(MITK_EXPORTED_TARGET_PROPERTIES "${MITK_EXPORTED_TARGET_PROPERTIES}
set_target_properties(${target_to_export} PROPERTIES MITK_AUTOLOAD_DIRECTORY \"${autoload_dir}\")")
  endif()

  get_target_property(deprecated_module ${target_to_export} MITK_MODULE_DEPRECATED_SINCE)
  if(deprecated_module)
    set(MITK_EXPORTED_TARGET_PROPERTIES "${MITK_EXPORTED_TARGET_PROPERTIES}
set_target_properties(${target_to_export} PROPERTIES MITK_MODULE_DEPRECATED_SINCE \"${deprecated_module}\")")
  endif()
endforeach()

# ---------------- External projects -----------------

get_property(MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS_CONFIG GLOBAL PROPERTY MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS)

set(MITK_CONFIG_EXTERNAL_PROJECTS )
#string(REPLACE "^^" ";" _mitk_external_projects ${MITK_EXTERNAL_PROJECTS})

foreach(ep ${MITK_EXTERNAL_PROJECTS})
  get_property(_components GLOBAL PROPERTY MITK_${ep}_COMPONENTS)
  set(MITK_CONFIG_EXTERNAL_PROJECTS "${MITK_CONFIG_EXTERNAL_PROJECTS}
set(MITK_USE_${ep} ${MITK_USE_${ep}})
set(MITK_${ep}_DIR \"${${ep}_DIR}\")
set(MITK_${ep}_COMPONENTS ${_components})
")
endforeach()

foreach(ep ${MITK_EXTERNAL_PROJECTS})
  get_property(_package GLOBAL PROPERTY MITK_${ep}_PACKAGE)
  get_property(_components GLOBAL PROPERTY MITK_${ep}_COMPONENTS)
  if(_components)
      set(_components_arg COMPONENTS \${_components})
  else()
    set(_components_arg)
  endif()

  if(_package)
    set(MITK_CONFIG_EXTERNAL_PROJECTS "${MITK_CONFIG_EXTERNAL_PROJECTS}
if(MITK_USE_${ep})
  set(${ep}_DIR \${MITK_${ep}_DIR})
  if(MITK_${ep}_COMPONENTS)
    mitkMacroFindDependency(${_package} COMPONENTS \${MITK_${ep}_COMPONENTS})
  else()
    mitkMacroFindDependency(${_package})
  endif()
endif()")
  endif()
endforeach()


# ---------------- Tools -----------------

configure_file(${MITK_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in
               ${MITK_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
configure_file(${MITK_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in
               ${MITK_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
configure_file(${MITK_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in
               ${MITK_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

# ---------------- Configure files -----------------

configure_file(mitkVersion.h.in ${MITK_BINARY_DIR}/mitkVersion.h)
configure_file(mitkConfig.h.in ${MITK_BINARY_DIR}/mitkConfig.h)

set(IPFUNC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipFunc)
set(UTILITIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities)

configure_file(mitkConfig.h.in ${MITK_BINARY_DIR}/mitkConfig.h)
configure_file(MITKConfig.cmake.in ${MITK_BINARY_DIR}/MITKConfig.cmake @ONLY)

write_basic_config_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${MITK_VERSION_STRING} COMPATIBILITY AnyNewerVersion)

# If we are under Windows, create two batch files which correctly
# set up the environment for the application and for Visual Studio
if(WIN32)
  include(mitkFunctionCreateWindowsBatchScript)

  set(VS_SOLUTION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.sln")
  foreach(VS_BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript("${MITK_SOURCE_DIR}/CMake/StartVS.bat.in"
      ${PROJECT_BINARY_DIR}/StartVS_${VS_BUILD_TYPE}.bat
      ${VS_BUILD_TYPE})
  endforeach()
endif(WIN32)


#-----------------------------------------------------------------------------
# MITK Applications
#-----------------------------------------------------------------------------

# This must come after MITKConfig.h was generated, since applications
# might do a find_package(MITK REQUIRED).
add_subdirectory(Applications)

#-----------------------------------------------------------------------------
# MITK Examples
#-----------------------------------------------------------------------------

if(MITK_BUILD_EXAMPLES)
  # This must come after MITKConfig.h was generated, since applications
  # might do a find_package(MITK REQUIRED).
  add_subdirectory(Examples)
endif()

#-----------------------------------------------------------------------------
# Print configuration summary
#-----------------------------------------------------------------------------

message("\n\n")
feature_summary(
  DESCRIPTION "------- FEATURE SUMMARY FOR ${PROJECT_NAME} -------"
  WHAT ALL
)

