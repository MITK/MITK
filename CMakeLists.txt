if(APPLE)
  # With XCode 4.3, the SDK location changed. Older CMake
  # versions are not able to find it.
  cmake_minimum_required(VERSION 2.8.8)
else()
  cmake_minimum_required(VERSION 2.8.4)
endif()

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
# Superbuild Option - Enabled by default
#-----------------------------------------------------------------------------

option(MITK_USE_SUPERBUILD "Build MITK and the projects it depends on via SuperBuild.cmake." ON)

if(MITK_USE_SUPERBUILD)
  project(MITK-superbuild)
  set(MITK_SOURCE_DIR ${PROJECT_SOURCE_DIR})
  set(MITK_BINARY_DIR ${PROJECT_BINARY_DIR})
else()
  project(MITK)
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
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

set(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
foreach(policy ${project_policies})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH
  ${MITK_SOURCE_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# CMake function(s) and macro(s)
#-----------------------------------------------------------------------------

include(mitkMacroEmptyExternalProject)
include(mitkFunctionGenerateProjectXml)
include(mitkFunctionSuppressWarnings)

SUPPRESS_VC_DEPRECATED_WARNINGS()

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

foreach(type LIBRARY RUNTIME ARCHIVE)
  # Make sure the directory exists
  if(DEFINED MITK_CMAKE_${type}_OUTPUT_DIRECTORY
     AND NOT EXISTS ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY})
    message("Creating directory MITK_CMAKE_${type}_OUTPUT_DIRECTORY: ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY}")
    file(MAKE_DIRECTORY "${MITK_CMAKE_${type}_OUTPUT_DIRECTORY}")
  endif()

  if(MITK_USE_SUPERBUILD)
    set(output_dir ${MITK_BINARY_DIR}/bin)
    if(NOT DEFINED MITK_CMAKE_${type}_OUTPUT_DIRECTORY)
      set(MITK_CMAKE_${type}_OUTPUT_DIRECTORY ${MITK_BINARY_DIR}/MITK-build/bin)
    endif()
  else()
    if(NOT DEFINED MITK_CMAKE_${type}_OUTPUT_DIRECTORY)
      set(output_dir ${MITK_BINARY_DIR}/bin)
    else()
      set(output_dir ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY})
    endif()
  endif()
  set(CMAKE_${type}_OUTPUT_DIRECTORY ${output_dir} CACHE INTERNAL "Single output directory for building all libraries.")
  mark_as_advanced(CMAKE_${type}_OUTPUT_DIRECTORY)
endforeach()

#-----------------------------------------------------------------------------
# Additional MITK Options (also shown during superbuild)
#-----------------------------------------------------------------------------

option(BUILD_SHARED_LIBS "Build MITK with shared libraries" ON)
option(WITH_COVERAGE "Enable/Disable coverage" OFF)
option(BUILD_TESTING "Test the project" ON)

option(MITK_BUILD_ALL_APPS "Build all MITK applications" OFF)
set(MITK_BUILD_TUTORIAL OFF CACHE INTERNAL "Deprecated! Use MITK_BUILD_EXAMPLES instead!")
option(MITK_BUILD_EXAMPLES "Build the MITK Examples" ${MITK_BUILD_TUTORIAL})
option(MITK_USE_Boost "Use the Boost C++ library" OFF)
option(MITK_USE_BLUEBERRY "Build the BlueBerry platform" ON)
option(MITK_USE_CTK "Use CTK in MITK" ${MITK_USE_BLUEBERRY})
option(MITK_USE_QT "Use Nokia's Qt library" ${MITK_USE_CTK})
option(MITK_USE_DCMTK "EXPERIMENTAL, superbuild only: Use DCMTK in MITK" ${MITK_USE_CTK})
option(MITK_DCMTK_BUILD_SHARED_LIBS "EXPERIMENTAL, superbuild only: build DCMTK as shared libs" OFF)
option(MITK_USE_OpenCV "Use Intel's OpenCV library" OFF)
option(MITK_USE_OpenCL "Use OpenCL GPU-Computing library" OFF)
option(MITK_USE_Python "Use Python wrapping in MITK" OFF)
set(MITK_USE_CableSwig ${MITK_USE_Python})

mark_as_advanced(MITK_BUILD_ALL_APPS
                 MITK_USE_CTK
                 MITK_USE_DCMTK
                )


if(MITK_USE_Boost)
  option(MITK_USE_SYSTEM_Boost "Use the system Boost" OFF)
  set(MITK_USE_Boost_LIBRARIES "" CACHE STRING "A semi-colon separated list of required Boost libraries")
endif()

if(MITK_USE_BLUEBERRY)
  option(MITK_BUILD_ALL_PLUGINS "Build all MITK plugins" OFF)
  mark_as_advanced(MITK_BUILD_ALL_PLUGINS)

  if(NOT MITK_USE_CTK)
    message("Forcing MITK_USE_CTK to ON because of MITK_USE_BLUEBERRY")
    set(MITK_USE_CTK ON CACHE BOOL "Use CTK in MITK" FORCE)
  endif()
endif()

if(MITK_USE_CTK)
  if(NOT MITK_USE_QT)
    message("Forcing MITK_USE_QT to ON because of MITK_USE_CTK")
    set(MITK_USE_QT ON CACHE BOOL "Use Nokia's Qt library in MITK" FORCE)
  endif()
  if(NOT MITK_USE_DCMTK)
    message("Setting MITK_USE_DCMTK to ON because DCMTK needs to be build for CTK")
    set(MITK_USE_DCMTK ON CACHE BOOL "Use DCMTK in MITK" FORCE)
  endif()
endif()

if(MITK_USE_QT)
  # find the package at the very beginning, so that QT4_FOUND is available
  find_package(Qt4 4.6.2 REQUIRED)
endif()


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

if(NOT MITK_ACCESSBYITK_DIMENSIONS)
  set(MITK_ACCESSBYITK_DIMENSIONS
      "2,3"
      CACHE STRING "List of dimensions used in AccessByItk and InstantiateAccessFunction macros")
endif()

#-----------------------------------------------------------------------------
# Additional CXX/C Flags
#-----------------------------------------------------------------------------

set(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C Flags")
mark_as_advanced(ADDITIONAL_C_FLAGS)
set(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX Flags")
mark_as_advanced(ADDITIONAL_CXX_FLAGS)

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
  return()
endif()

#*****************************************************************************
#****************************  END OF SUPERBUILD  ****************************
#*****************************************************************************

#-----------------------------------------------------------------------------
# CMake function(s) and macro(s)
#-----------------------------------------------------------------------------

include(CheckCXXSourceCompiles)
include(mitkFunctionCheckCompilerFlags)
include(mitkFunctionGetGccVersion)
include(MacroParseArguments)
include(mitkFunctionSuppressWarnings) # includes several functions
include(mitkFunctionOrganizeSources)
include(mitkFunctionGetVersion)
include(mitkFunctionGetVersionDescription)
include(mitkFunctionCreateWindowsBatchScript)
include(mitkFunctionInstallProvisioningFiles)
include(mitkFunctionInstallAutoLoadModules)
include(mitkFunctionCompileSnippets)
include(mitkMacroCreateModuleConf)
include(mitkMacroCreateModule)
include(mitkMacroCheckModule)
include(mitkMacroCreateModuleTests)
include(mitkFunctionAddCustomModuleTest)
include(mitkMacroUseModule)
include(mitkMacroMultiplexPicType)
include(mitkMacroInstall)
include(mitkMacroInstallHelperApp)
include(mitkMacroInstallTargets)
include(mitkMacroGenerateToolsLibrary)
include(mitkMacroGetLinuxDistribution)

#-----------------------------------------------------------------------------
# Prerequesites
#-----------------------------------------------------------------------------

find_package(ITK REQUIRED)
find_package(VTK REQUIRED)

if(ITK_USE_SYSTEM_GDCM)
  find_package(GDCM PATHS ${ITK_GDCM_DIR} REQUIRED)
endif()

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
      set(MACOSX_BUNDLE_NAMES ${MACOSX_BUNDLE_NAMES} ${app_name})
    endif()
  endforeach()

endif()

#-----------------------------------------------------------------------------
# Set symbol visibility Flags
#-----------------------------------------------------------------------------

# MinGW does not export all symbols automatically, so no need to set flags
if(CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
  set(VISIBILITY_CXX_FLAGS ) #"-fvisibility=hidden -fvisibility-inlines-hidden")
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

set(MITK_C_FLAGS "${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}")
set(MITK_CXX_FLAGS "${VISIBILITY_CXX_FLAGS} ${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}")

include(mitkSetupC++0xVariables)

set(cflags )
if(WIN32)
  set(cflags "${cflags} -DPOCO_NO_UNWINDOWS -DWIN32_LEAN_AND_MEAN")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(cflags "${cflags} -Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings")

  mitkFunctionCheckCompilerFlags("-fdiagnostics-show-option" cflags)
  mitkFunctionCheckCompilerFlags("-Wl,--no-undefined" cflags)
  mitkFunctionCheckCompilerFlags("-Wl,--as-needed" cflags)

  if(MITK_USE_C++0x)
    mitkFunctionCheckCompilerFlags("-std=c++0x" MITK_CXX_FLAGS)
  endif()

  mitkFunctionGetGccVersion(${CMAKE_CXX_COMPILER} GCC_VERSION)
  # With older version of gcc supporting the flag -fstack-protector-all, an extra dependency to libssp.so
  # is introduced. If gcc is smaller than 4.4.0 and the build type is Release let's not include the flag.
  # Doing so should allow to build package made for distribution using older linux distro.
  if(${GCC_VERSION} VERSION_GREATER "4.4.0" OR (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ${GCC_VERSION} VERSION_LESS "4.4.0"))
    mitkFunctionCheckCompilerFlags("-fstack-protector-all" cflags)
  endif()
  if(MINGW)
    # suppress warnings about auto imported symbols
    set(MITK_CXX_FLAGS "-Wl,--enable-auto-import ${MITK_CXX_FLAGS}")
    # we need to define a Windows version
    set(MITK_CXX_FLAGS "-D_WIN32_WINNT=0x0500 ${MITK_CXX_FLAGS}")
  endif()

  #set(MITK_CXX_FLAGS "-Woverloaded-virtual -Wold-style-cast -Wstrict-null-sentinel -Wsign-promo ${MITK_CXX_FLAGS}")
  set(MITK_CXX_FLAGS "-Woverloaded-virtual -Wstrict-null-sentinel ${MITK_CXX_FLAGS}")
  set(MITK_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 ${MITK_CXX_FLAGS_RELEASE}")

endif()

set(MITK_C_FLAGS "${cflags} ${MITK_C_FLAGS}")
set(MITK_CXX_FLAGS "${cflags} ${MITK_CXX_FLAGS}")

#-----------------------------------------------------------------------------
# MITK Packages
#-----------------------------------------------------------------------------

set(MITK_MODULES_PACKAGE_DEPENDS_DIR ${MITK_SOURCE_DIR}/CMake/PackageDepends)
set(MODULES_PACKAGE_DEPENDS_DIRS ${MITK_MODULES_PACKAGE_DEPENDS_DIR})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
  enable_testing()
  include(CTest)
  mark_as_advanced(TCL_TCLSH DART_ROOT)

  option(MITK_ENABLE_GUI_TESTING OFF "Enable the MITK GUI tests")

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
# Compile Utilities and set-up MITK variables
#-----------------------------------------------------------------------------

include(mitkSetupVariables)

  #-----------------------------------------------------------------------------
  # Cleanup
  #-----------------------------------------------------------------------------

  file(GLOB _MODULES_CONF_FILES ${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME}/*.cmake)
  if(_MODULES_CONF_FILES)
    file(REMOVE ${_MODULES_CONF_FILES})
  endif()

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

  add_subdirectory(BlueBerry)

  set(BlueBerry_DIR ${CMAKE_CURRENT_BINARY_DIR}/BlueBerry
      CACHE PATH "The directory containing a CMake configuration file for BlueBerry" FORCE)

  include(mitkMacroCreateCTKPlugin)

endif()

#-----------------------------------------------------------------------------
# Set C/CXX Flags for MITK code
#-----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MITK_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MITK_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MITK_C_FLAGS}")

if(MITK_USE_QT)
  add_definitions(-DQWT_DLL)
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

link_directories(${MITK_LINK_DIRECTORIES})

add_subdirectory(Core)
add_subdirectory(Modules)

if(MITK_USE_BLUEBERRY)

  find_package(BlueBerry REQUIRED)

  set(MITK_DEFAULT_SUBPROJECTS MITK-Plugins)

  # Plug-in testing (needs some work to be enabled again)
  if(BUILD_TESTING)
    include(berryTestingHelpers)

    set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp")
    get_target_property(_is_macosx_bundle CoreApp MACOSX_BUNDLE)
    if(APPLE AND _is_macosx_bundle)
      set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp.app/Contents/MacOS/CoreApp")
    endif()

    set(BLUEBERRY_TEST_APP_ID "org.mitk.qt.coreapplication")
  endif()

  include("${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PluginList.cmake")
  set(mitk_plugins_fullpath )
  foreach(mitk_plugin ${MITK_EXT_PLUGINS})
    list(APPEND mitk_plugins_fullpath Plugins/${mitk_plugin})
  endforeach()

  if(EXISTS ${MITK_PRIVATE_MODULES}/PluginList.cmake)
    include(${MITK_PRIVATE_MODULES}/PluginList.cmake)

    foreach(mitk_plugin ${MITK_PRIVATE_PLUGINS})
      list(APPEND mitk_plugins_fullpath ${MITK_PRIVATE_MODULES}/${mitk_plugin})
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
    list(APPEND mitk_apps_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/Applications/${mitk_app}")
  endforeach()

  ctkMacroSetupPlugins(${mitk_plugins_fullpath}
                       BUILD_OPTION_PREFIX MITK_BUILD_
                       APPS ${mitk_apps_fullpath}
                       BUILD_ALL ${MITK_BUILD_ALL_PLUGINS}
                       COMPACT_OPTIONS)

  set(MITK_PLUGIN_USE_FILE "${MITK_BINARY_DIR}/MitkPluginUseFile.cmake")
  if(${PROJECT_NAME}_PLUGIN_LIBRARIES)
    ctkFunctionGeneratePluginUseFile(${MITK_PLUGIN_USE_FILE})
  else()
    file(REMOVE ${MITK_PLUGIN_USE_FILE})
    set(MITK_PLUGIN_USE_FILE )
  endif()

endif()

# Construct a list of paths containing runtime directories
# for MITK applications on Windows
set(MITK_RUNTIME_PATH
    "${VTK_LIBRARY_DIRS}/%VS_BUILD_TYPE%;${ITK_LIBRARY_DIRS}/%VS_BUILD_TYPE%;${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/%VS_BUILD_TYPE%"
    )

if(QT4_FOUND)
  set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${QT_LIBRARY_DIR}/../bin")
endif()

if(MITK_USE_BLUEBERRY)
  set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${CTK_RUNTIME_LIBRARY_DIRS}/%VS_BUILD_TYPE%")
  if(DEFINED CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY)
    if(IS_ABSOLUTE "${CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY}")
      set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY}/%VS_BUILD_TYPE%")
    else()
      set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CTK_PLUGIN_RUNTIME_OUTPUT_DIRECTORY}/%VS_BUILD_TYPE%")
    endif()
  else()
    set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/%VS_BUILD_TYPE%")
  endif()
endif()

if(GDCM_DIR)
  set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${GDCM_DIR}/bin/%VS_BUILD_TYPE%")
endif()

if(OpenCV_DIR)
  set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${OpenCV_DIR}/bin/%VS_BUILD_TYPE%")
endif()

# DCMTK is statically build
#if(DCMTK_DIR)
#  set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${DCMTK_DIR}/bin/%VS_BUILD_TYPE%")
#endif()

if(MITK_USE_Boost AND MITK_USE_Boost_LIBRARIES AND NOT MITK_USE_SYSTEM_Boost)
  set(MITK_RUNTIME_PATH "${MITK_RUNTIME_PATH};${Boost_LIBRARY_DIRS}")
endif()

#-----------------------------------------------------------------------------
# Python Wrapping
#-----------------------------------------------------------------------------

set(MITK_WRAPPING_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping)
set(MITK_WRAPPING_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Wrapping)
option(MITK_USE_Python "Build cswig Python wrapper support (requires CableSwig)." OFF)
if(MITK_USE_Python)
  add_subdirectory(Wrapping)
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

# This is for installation support of external projects depending on
# MITK plugins and modules. The export file should not be used for linking to MITK
# libraries without using LINK_DIRECTORIES, since the exports are incomplete
# yet (depending libraries are not exported).

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
endforeach()

configure_file(${MITK_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in
               ${MITK_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
configure_file(${MITK_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in
               ${MITK_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
configure_file(${MITK_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in
               ${MITK_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

set(VISIBILITY_AVAILABLE 0)
set(visibility_test_flag "")
mitkFunctionCheckCompilerFlags("-fvisibility=hidden" visibility_test_flag)
if(visibility_test_flag)
  # The compiler understands -fvisiblity=hidden (probably gcc >= 4 or Clang)
  set(VISIBILITY_AVAILABLE 1)
endif()
configure_file(mitkExportMacros.h.in ${MITK_BINARY_DIR}/mitkExportMacros.h)

configure_file(mitkVersion.h.in ${MITK_BINARY_DIR}/mitkVersion.h)
configure_file(mitkConfig.h.in ${MITK_BINARY_DIR}/mitkConfig.h)

set(VECMATH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/vecmath)
set(IPFUNC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipFunc)
set(UTILITIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities)

file(GLOB _MODULES_CONF_FILES RELATIVE ${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME} ${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME}/*.cmake)
set(MITK_MODULE_NAMES)
foreach(_module ${_MODULES_CONF_FILES})
  string(REPLACE Config.cmake "" _module_name ${_module})
  list(APPEND MITK_MODULE_NAMES ${_module_name})
endforeach()

configure_file(mitkConfig.h.in ${MITK_BINARY_DIR}/mitkConfig.h)
configure_file(MITKConfig.cmake.in ${MITK_BINARY_DIR}/MITKConfig.cmake @ONLY)

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

