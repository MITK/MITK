set(MITK_CMAKE_MINIMUM_REQUIRED_VERSION 3.2)
cmake_minimum_required(VERSION ${MITK_CMAKE_MINIMUM_REQUIRED_VERSION})

#-----------------------------------------------------------------------------
# See http://www.cmake.org/cmake/help/v3.2/manual/cmake-policies.7.html for details
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
  project(MITK VERSION 2016.03.99)
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
# Check miminum Mac OS X version
#-----------------------------------------------------------------------------
# The minimum supported Mac OS X version is 10.9. If you use a version less than 10.9, there is no guarantee that the build still works.
if(APPLE)
  exec_program(sw_vers ARGS -productVersion OUTPUT_VARIABLE osx_version)
  if (osx_version VERSION_LESS "10.9")
    message(WARNING "Detected OS X version \"${osx_version}\" is not supported anymore. Minimum required OS X version is 10.9 or greater.")
  endif()
  if (CMAKE_OSX_DEPLOYMENT_TARGET AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 10.9)
    message(WARNING "Detected OS X deployment target \"${CMAKE_OSX_DEPLOYMENT_TARGET}\" is not supported anymore. Minimum required OS X version is 10.9 or greater.")
  endif()
endif()

#-----------------------------------------------------------------------------
# Check miminum compiler versions
#-----------------------------------------------------------------------------

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # require at least gcc 4.7.3 as provided by ppa:ubuntu-toolchain-r/test for Ubuntu 12.04
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7.3)
    message(FATAL_ERROR "GCC version must be at least 4.7.3
If you are using Ubuntu 12.04, you can easily install gcc and g++ 4.7.3 (or any later version available) in addition to your version ${CMAKE_CXX_COMPILER_VERSION}:
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo apt-get update
  sudo apt-get install gcc-4.7 g++-4.7
Make sure to explicitly specify these compilers when configuring MITK:
  CMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc-4.7
  CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++-4.7
For more information on the proposed PPA see the Toolchain Updates section of https://wiki.ubuntu.com/ToolChain.")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # require at least clang 3.4 as provided by Ubuntu 12.04
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
    message(FATAL_ERROR "Clang version must be at least 3.4")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # require at least clang 5.0
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "Apple Clang version must be at least 5.0")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # require at least Visual Studio 2012
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 17.0.61030.0)
    message(FATAL_ERROR "Microsoft Visual Studio 2012 Update 4 or newer required (MSVC 17.0.61030.0)")
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

set(CMAKE_CXX_EXTENSIONS 0)
set(CMAKE_CXX_STANDARD ${MITK_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED 1)

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
    set(_src_dir_length_max 34) # _src_dir_length_max - strlen(ep/src/ITK-build)
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

env_option(MITK_USE_QT "Use Qt library" ON)
env_option(MITK_USE_QT_WEBENGINE "Use Qt WebEngine library" ON)

if(MITK_USE_QT)
  set(MITK_QT5_MINIMUM_VERSION 5.6.0)
  set(MITK_QT5_COMPONENTS Concurrent OpenGL PrintSupport Script Sql Svg Widgets Xml XmlPatterns UiTools Help LinguistTools)
  if(MITK_USE_QT_WEBENGINE)
    set(MITK_QT5_COMPONENTS ${MITK_QT5_COMPONENTS} WebEngineWidgets)
  endif()
  if(APPLE)
    set(MITK_QT5_COMPONENTS ${MITK_QT5_COMPONENTS} DBus)
  endif()
  find_package(Qt5 ${MITK_QT5_MINIMUM_VERSION} COMPONENTS ${MITK_QT5_COMPONENTS} REQUIRED)
  if(Qt5_DIR)
    get_filename_component(_Qt5_DIR "${Qt5_DIR}/../../../" ABSOLUTE)
    list(FIND CMAKE_PREFIX_PATH "${_Qt5_DIR}" _result)
    if(_result LESS 0)
      set(CMAKE_PREFIX_PATH "${_Qt5_DIR};${CMAKE_PREFIX_PATH}" CACHE PATH "" FORCE)
    endif()
  endif()
elseif(MITK_USE_QT_WEBENGINE)
  set(MITK_USE_QT_WEBENGINE OFF)
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

mitkFunctionAddExternalProject(NAME Poco        ON  COMPONENTS Foundation Net Util XML Zip)
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

mitkFunctionAddExternalProject(NAME HDF5        OFF ADVANCED)

# -----------------------------------------
# The following external projects must be
# ordered according to their
# inter-dependencies

mitkFunctionAddExternalProject(NAME SWIG      OFF ADVANCED NO_PACKAGE DEPENDS PCRE)
mitkFunctionAddExternalProject(NAME Python    OFF          NO_PACKAGE DEPENDS SWIG DOC "Use Python wrapping in MITK")
mitkFunctionAddExternalProject(NAME Numpy     OFF ADVANCED NO_PACKAGE)
mitkFunctionAddExternalProject(NAME OpenCV    OFF)
mitkFunctionAddExternalProject(NAME Vigra     OFF          DEPENDS HDF5)

# These are "hard" dependencies and always set to ON
mitkFunctionAddExternalProject(NAME VTK       ON           NO_CACHE)
mitkFunctionAddExternalProject(NAME Boost     ON           NO_CACHE)
mitkFunctionAddExternalProject(NAME ITK       ON           NO_CACHE)

mitkFunctionAddExternalProject(NAME SimpleITK OFF          DEPENDS ITK GDCM SWIG)
mitkFunctionAddExternalProject(NAME ACVD      OFF          DOC "Use Approximated Centroidal Voronoi Diagrams")
mitkFunctionAddExternalProject(NAME CTK       ON           DEPENDS QT DCMTK DOC "Use CTK in MITK")
mitkFunctionAddExternalProject(NAME Rasqal    OFF          DEPENDS Raptor2 PCRE ADVANCED)
mitkFunctionAddExternalProject(NAME Redland   OFF          DEPENDS Rasqal DOC "Use the Redland RDF library")
mitkFunctionAddExternalProject(NAME SOFA      OFF          DEPENDS GLUT Boost DOC "Use Simulation Open Framework Architecture")
mitkFunctionAddExternalProject(NAME VMTK      OFF          DEPENDS ITK VTK)

if(MITK_USE_QT)
  mitkFunctionAddExternalProject(NAME Qwt     ON  ADVANCED DEPENDS QT)
endif()

# -----------------------------------------
# Other MITK_USE_* options not related to
# external projects build via the
# MITK superbuild

env_option(MITK_USE_BLUEBERRY "Build the BlueBerry platform" ON)
env_option(MITK_USE_OpenCL "Use OpenCL GPU-Computing library" OFF)

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
set_property(CACHE MITK_BUILD_CONFIGURATION PROPERTY STRINGS ${_buildConfigs})

mitkFunctionEnableBuildConfiguration()

mitkFunctionCreateWhitelistPaths(MITK)
mitkFunctionFindWhitelists(MITK)

# -----------------------------------------
# Custom dependency logic

option(MITK_USE_SYSTEM_Boost "Use the system Boost" OFF)
set(MITK_USE_Boost_LIBRARIES "" CACHE STRING "A semi-colon separated list of required Boost libraries")

if(MITK_USE_SOFA)
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

# sanity check for supported Qt version. Only >= 5.3 is supported by CTK/PythonQt
if(MITK_USE_QT AND MITK_USE_Python)
  set(minimum_required_python_qt5_version "5.3.0")
  find_package(Qt5 COMPONENTS Core REQUIRED)

  if(${Qt5Core_VERSION_STRING} VERSION_LESS ${minimum_required_python_qt5_version})
    message(WARNING "Can't build MITK Python with Qt version < ${minimum_required_python_qt5_version}. Disabling Python support")
    set(MITK_USE_Python OFF CACHE BOOL "Use python wrapping in MITK" FORCE)
  endif()
endif()

# only windows can't build python in debug mode
if(MITK_USE_Python AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND WIN32)
  message(WARNING "Disabling Python support. Building MITK Python in debug mode on Windowsis not supported!")
  set(MITK_USE_Python OFF CACHE BOOL "Use python wrapping in MITK" FORCE)
  set(MITK_USE_Numpy OFF CACHE BOOL "Use Numpy" FORCE)
  set(MITK_USE_SimpleITK OFF CACHE BOOL "Use SimpleITK" FORCE)
elseif(MITK_USE_Python)
  set(MITK_USE_ZLIB ON)
  if(NOT MITK_USE_Numpy)
    message("> Forcing MITK_USE_Numpy to ON because of MITK_USE_Python")
    set(MITK_USE_Numpy ON CACHE BOOL "Use Numpy" FORCE)
  endif()
  if(NOT MITK_USE_SimpleITK)
    message("> Forcing MITK_USE_SimpleITK to ON because of MITK_USE_Python")
    set(MITK_USE_SimpleITK ON CACHE BOOL "Use SimpleITK" FORCE)
  endif()

  option(MITK_USE_SYSTEM_PYTHON "Use the system python runtime" OFF)
  if(MITK_USE_SYSTEM_PYTHON)
    find_package(PythonLibs REQUIRED)
    find_package(PythonInterp REQUIRED)
  endif()
elseif(MITK_USE_Python AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND WIN32)
  message(WARNING "Disabling Python support. Building MITK Python in debug mode on Windowsis not supported!")
  set(MITK_USE_Python OFF CACHE BOOL "Use python wrapping in MITK" FORCE)
endif()

if(BUILD_TESTING AND NOT MITK_USE_CppUnit)
  message("> Forcing MITK_USE_CppUnit to ON because BUILD_TESTING=ON")
  set(MITK_USE_CppUnit ON CACHE BOOL "Use CppUnit for unit tests" FORCE)
endif()

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

# Set up data downloads URL
if(NOT MITK_DATA_DOWNLOAD_PREFIX_URL)
  set(MITK_DATA_DOWNLOAD_PREFIX_URL http://mitk.org/download/data)
endif()

#-----------------------------------------------------------------------------
# Project.xml
#-----------------------------------------------------------------------------

# A list of topologically ordered targets
set(CTEST_PROJECT_SUBPROJECTS)

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

include(mitkFunctionAddCustomModuleTest)
include(mitkFunctionCheckModuleDependencies)
include(mitkFunctionCompileSnippets)
include(mitkFunctionConfigureVisualStudioUserProjectFile)
include(mitkFunctionConvertXPSchema)
include(mitkFunctionCreateBlueBerryApplication)
include(mitkFunctionCreateCommandLineApp)
include(mitkFunctionCreateModule)
include(mitkFunctionCreatePlugin)
include(mitkFunctionCreateProvisioningFile)
include(mitkFunctionCreateWindowsBatchScript)
include(mitkFunctionGetLibrarySearchPaths)
include(mitkFunctionGetVersion)
include(mitkFunctionGetVersionDescription)
include(mitkFunctionInstallAutoLoadModules)
include(mitkFunctionInstallCTKPlugin)
include(mitkFunctionInstallProvisioningFiles)
include(mitkFunctionInstallThirdPartyCTKPlugins)
include(mitkFunctionOrganizeSources)
include(mitkFunctionTestPlugin)
include(mitkFunctionUseModules)
include(mitkMacroConfigureItkPixelTypes)
include(mitkMacroCreateExecutable)
include(mitkMacroCreateModuleTests)
include(mitkMacroGenerateToolsLibrary)
include(mitkMacroGetLinuxDistribution)
include(mitkMacroGetPMDPlatformString)
include(mitkMacroInstall)
include(mitkMacroInstallHelperApp)
include(mitkMacroInstallTargets)
include(mitkMacroMultiplexPicType)

# Deprecated
include(mitkMacroCreateCTKPlugin)

#-----------------------------------------------------------------------------
# Global CMake variables
#-----------------------------------------------------------------------------

# Required and enabled C++11 features for all MITK code.
# These are added as PUBLIC compile features to all MITK modules.
set(MITK_CXX_FEATURES
  cxx_auto_type
  cxx_decltype
  cxx_enum_forward_declarations
  cxx_extended_friend_declarations
  cxx_extern_templates
  cxx_final
  cxx_lambdas
  cxx_local_type_template_args
  cxx_long_long_type
  cxx_nullptr
  cxx_override
  cxx_range_for
  cxx_right_angle_brackets
  cxx_rvalue_references
  cxx_static_assert
  cxx_strong_enums
  cxx_template_template_parameters
  cxx_trailing_return_types
  cxx_variadic_macros
)

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

# Look for optional Doxygen package
find_package(Doxygen)

option(BLUEBERRY_DEBUG_SMARTPOINTER "Enable code for debugging smart pointers" OFF)
mark_as_advanced(BLUEBERRY_DEBUG_SMARTPOINTER)

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

# MITK_VERSION
set(MITK_VERSION_STRING "${MITK_VERSION_MAJOR}.${MITK_VERSION_MINOR}.${MITK_VERSION_PATCH}")
if(MITK_VERSION_PATCH STREQUAL "99")
  set(MITK_VERSION_STRING "${MITK_VERSION_STRING}-${MITK_REVISION_SHORTID}")
endif()

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

find_package(OpenMP)

if (OPENMP_FOUND)
  set(MITK_C_FLAGS "${MITK_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(MITK_CXX_FLAGS "${MITK_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(WIN32)
  set(MITK_CXX_FLAGS "${MITK_CXX_FLAGS} -D_WIN32_WINNT=0x0501 -DPOCO_NO_UNWINDOWS -DWIN32_LEAN_AND_MEAN -DNOMINMAX")
  mitkFunctionCheckCompilerFlags("/wd4005" MITK_CXX_FLAGS) # warning C4005: macro redefinition
  mitkFunctionCheckCompilerFlags("/wd4231" MITK_CXX_FLAGS) # warning C4231: nonstandard extension used : 'extern' before template explicit instantiation
  # the following line should be removed after fixing bug 17637
  mitkFunctionCheckCompilerFlags("/wd4316" MITK_CXX_FLAGS) # warning C4316: object alignment on heap
  mitkFunctionCheckCompilerFlags("/wd4180" MITK_CXX_FLAGS) # warning C4180: qualifier applied to function type has no meaning
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
  mitkFunctionCheckCAndCXXCompilerFlags("-fstack-protector-all" MITK_C_FLAGS MITK_CXX_FLAGS)

  if(MINGW)
    # suppress warnings about auto imported symbols
    set(MITK_SHARED_LINKER_FLAGS "-Wl,--enable-auto-import ${MITK_SHARED_LINKER_FLAGS}")
  endif()

  set(MITK_CXX_FLAGS_RELEASE "-U_FORTIFY_SOURCES -D_FORTIFY_SOURCE=2 ${MITK_CXX_FLAGS_RELEASE}")
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
set(Boost_ADDITIONAL_VERSIONS "1.60" "1.60.0")

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
  if(MITK_USE_Numpy)
    find_package(Numpy REQUIRED)
  endif()
endif()

if(MITK_USE_SOFA)
  # The SOFAConfig.cmake file does not provide exported targets or
  # libraries with absolute paths, hence we need to make the link
  # directories globally available until the SOFAConfig.cmake file
  # supports a proper mechanism for handling targets.
  # The same code is needed in MITKConfig.cmake.
  link_directories(${SOFA_LIBRARY_DIRS})
endif()

# Same as SOFA above
link_directories(${Boost_LIBRARY_DIRS})

if(MITK_USE_OpenIGTLink)
  # Same as SOFA above
  link_directories(${OpenIGTLink_LIBRARY_DIRS})
endif()

if(MITK_USE_SimpleITK)
  link_directories(${SimpleITK_LIBRARY_DIRS})
endif()

if(MITK_USE_OpenCL)
  find_package(OpenCL REQUIRED)
endif()

# Qt support
if(MITK_USE_QT)
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

  find_program(QT_HELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator qhelpgenerator-qt5 qhelpgenerator5
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  find_program(QT_COLLECTIONGENERATOR_EXECUTABLE
    NAMES qcollectiongenerator qcollectiongenerator-qt5 qcollectiongenerator5
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  find_program(QT_ASSISTANT_EXECUTABLE
    NAMES assistant assistant-qt5 assistant5
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  find_program(QT_XMLPATTERNS_EXECUTABLE
    NAMES xmlpatterns
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  mark_as_advanced(QT_HELPGENERATOR_EXECUTABLE
                   QT_COLLECTIONGENERATOR_EXECUTABLE
                   QT_ASSISTANT_EXECUTABLE
                   QT_XMLPATTERNS_EXECUTABLE
                  )

  if(MITK_USE_BLUEBERRY)
    option(BLUEBERRY_USE_QT_HELP "Enable support for integrating plugin documentation into Qt Help" ${DOXYGEN_FOUND})
    mark_as_advanced(BLUEBERRY_USE_QT_HELP)

    # Sanity checks for in-application BlueBerry plug-in help generation
    if(BLUEBERRY_USE_QT_HELP)
      set(_force_blueberry_use_qt_help_to_off 0)
      if(NOT DOXYGEN_FOUND)
        message("> Forcing BLUEBERRY_USE_QT_HELP to OFF because Doxygen was not found.")
        set(_force_blueberry_use_qt_help_to_off 1)
      endif()
      if(DOXYGEN_FOUND AND DOXYGEN_VERSION VERSION_LESS 1.8.7)
        message("> Forcing BLUEBERRY_USE_QT_HELP to OFF because Doxygen version 1.8.7 or newer not found.")
        set(_force_blueberry_use_qt_help_to_off 1)
      endif()

      if(NOT QT_HELPGENERATOR_EXECUTABLE)
        message("> Forcing BLUEBERRY_USE_QT_HELP to OFF because QT_HELPGENERATOR_EXECUTABLE is empty.")
        set(_force_blueberry_use_qt_help_to_off 1)
      endif()

      if(NOT MITK_USE_QT_WEBENGINE)
        message("> Forcing BLUEBERRY_USE_QT_HELP to OFF because MITK_USE_QT_WEBENGINE is OFF.")
        set(_force_blueberry_use_qt_help_to_off 1)
      endif()

      if(NOT QT_XMLPATTERNS_EXECUTABLE)
        message("You have enabled Qt Help support, but QT_XMLPATTERNS_EXECUTABLE is empty")
        set(_force_blueberry_use_qt_help_to_off 1)
      endif()

      if(_force_blueberry_use_qt_help_to_off)
        set(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating plugin documentation into Qt Help" FORCE)
      endif()
    endif()

    if(BLUEBERRY_QT_HELP_REQUIRED AND NOT BLUEBERRY_USE_QT_HELP)
      message(FATAL_ERROR "BLUEBERRY_USE_QT_HELP is required to be set to ON")
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

  # Initial cache for ProjectTemplate and PluginGenerator tests
  configure_file(
     CMake/mitkTestInitialCache.txt.in
     ${MITK_BINARY_DIR}/mitkTestInitialCache.txt
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
    include(mitkTestProjectTemplate)
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

add_subdirectory(Utilities)

add_subdirectory(Modules)

if(MITK_USE_BLUEBERRY)

  set(BLUEBERRY_XPDOC_OUTPUT_DIR ${MITK_DOXYGEN_OUTPUT_DIR}/html/extension-points/html/)

  set(MITK_DEFAULT_SUBPROJECTS MITK-Plugins)

  # Plug-in testing (needs some work to be enabled again)
  if(BUILD_TESTING)
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
  mitkFunctionWhitelistPlugins(MITK MITK_PLUGINS)

  set(mitk_plugins_fullpath "")
  foreach(mitk_plugin ${MITK_PLUGINS})
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
    # extract option_name
    string(REPLACE "^^" "\\;" target_info ${mitk_app})
    set(target_info_list ${target_info})
    list(GET target_info_list 0 directory_name)
    list(GET target_info_list 1 option_name)
    if(${option_name})
      list(APPEND mitk_apps_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/Applications/${directory_name}^^${option_name}")
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

if(DOXYGEN_FOUND)
  add_subdirectory(Documentation)
endif()

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
    list(GET target_info_list 2 executable_name)
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
    list(APPEND CPACK_CREATE_DESKTOP_LINKS "${executable_name}")
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
  if(MITK_USE_HDF5)
    list(APPEND MITK_LIBRARY_DIRS ${HDF5_DIR}/install/bin)
    message(STATUS "MITK-Runtime " ${MITK_RUNTIME_PATH})
  endif(MITK_USE_HDF5)
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

#------------------------------------------------------------------------------
# Copy file font.
#------------------------------------------------------------------------------
file(COPY Fonts/DejaVuSans.ttf DESTINATION ${PROJECT_BINARY_DIR}/bin/Debug/Fonts)
file(COPY Fonts/DejaVuSans.ttf DESTINATION ${PROJECT_BINARY_DIR}/bin/Release/Fonts)
file(COPY Fonts/DejaVuSans.ttf DESTINATION ${PROJECT_BINARY_DIR}/bin/RelWithDebInfo/Fonts)
file(COPY Fonts/DejaVuSans.ttf DESTINATION ${PROJECT_BINARY_DIR}/bin/MinSizeRel/Fonts)

#-----------------------------------------------------------------------------
# Print configuration summary
#-----------------------------------------------------------------------------

message("\n\n")
feature_summary(
  DESCRIPTION "------- FEATURE SUMMARY FOR ${PROJECT_NAME} -------"
  WHAT ALL
)
