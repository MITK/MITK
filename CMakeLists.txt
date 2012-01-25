cmake_minimum_required(VERSION 2.8.4)

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
# CMake Function(s) and Macro(s)
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
      SET(output_dir ${MITK_BINARY_DIR}/bin)
    else()
      SET(output_dir ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY})
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

# some targets in Utilities also depend on Qt. Use this option
# to decide if they should be build
option(MITK_USE_QT "Use Trolltech's Qt library" ON)
if(MITK_USE_QT)
  # find the package at the very beginning, so that QT4_FOUND is available
  find_package(Qt4 4.6.0 REQUIRED)
endif()

option(MITK_BUILD_ALL_PLUGINS "Build all MITK plugins" OFF)
option(MITK_BUILD_ALL_APPS "Build all MITK applications" OFF)
option(MITK_BUILD_TUTORIAL "Build the MITK tutorial" ON)
option(MITK_USE_Boost "Use the Boost C++ library" OFF)
if(MITK_USE_Boost)
  option(MITK_USE_SYSTEM_Boost "Use the system Boost" OFF)
  set(MITK_USE_Boost_LIBRARIES "" CACHE STRING "A semi-colon separated list of required Boost libraries")
endif()
option(MITK_USE_BLUEBERRY "Build the BlueBerry platform" ON)
option(MITK_USE_CTK "Use CTK in MITK" ${MITK_USE_BLUEBERRY})
option(MITK_USE_DCMTK "EXEPERIMENTAL, superbuild only: Use DCMTK in MITK" OFF)
option(MITK_USE_OpenCV "Use Intel's OpenCV library" OFF)
option(MITK_USE_Python "Use Python wrapping in MITK" OFF)
  
mark_as_advanced(MITK_BUILD_ALL_PLUGINS
                 MITK_BUILD_ALL_APPS
                 MITK_USE_CTK
                 MITK_USE_DCMTK
                 )

set(MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES
    "int, unsigned int, short, unsigned short, char, unsigned char"
    CACHE STRING "List of integral pixel types used in AccessByItk and InstantiateAccessFunction macros")

set(MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES
    "double, float"
    CACHE STRING "List of floating pixel types used in AccessByItk and InstantiateAccessFunction macros")

set(MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES
    ""
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
if(MITK_USE_BLUEBERRY AND NOT MITK_USE_CTK)
  MESSAGE(FATAL_ERROR "BlueBerry depends on CTK. Please set MITK_USE_CTK to ON.")
endif()

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
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------

include(CheckCXXSourceCompiles)
include(mitkFunctionCheckCompilerFlags)
include(mitkFunctionGetGccVersion)
include(MacroParseArguments)
include(mitkFunctionSuppressWarnings) # includes several functions
include(mitkFunctionOrganizeSources)
include(mitkFunctionGetVersion)
include(mitkFunctionCreateWindowsBatchScript)
include(mitkFunctionInstallProvisioningFiles)
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

# ASK THE USER TO SHOW THE CONSOLE WINDOW FOR CoreApp and ExtApp
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

#-----------------------------------------------------------------------------
# Installation preparation
#
# These should be set before any MITK install macros are used
#-----------------------------------------------------------------------------

# on Mac OSX all BlueBerry plugins get copied into every
# application bundle (.app directory) specified here
if(APPLE)
  if(MITK_BUILD_org.mitk.gui.qt.extapplication)
    set(MACOSX_BUNDLE_NAMES ${MACOSX_BUNDLE_NAMES} ExtApp)
  endif()
  if(MITK_BUILD_org.mitk.gui.qt.application)
    set(MACOSX_BUNDLE_NAMES ${MACOSX_BUNDLE_NAMES} CoreApp)
  endif()
endif(APPLE)

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

if(CMAKE_COMPILER_IS_GNUCXX)
  set(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings -D_FORTIFY_SOURCE=2")

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

  set(MITK_C_FLAGS "${cflags} ${MITK_C_FLAGS}")
  #set(MITK_CXX_FLAGS "${cflags} -Woverloaded-virtual -Wold-style-cast -Wstrict-null-sentinel -Wsign-promo ${MITK_CXX_FLAGS}")
  set(MITK_CXX_FLAGS "${cflags} -Woverloaded-virtual -Wstrict-null-sentinel ${MITK_CXX_FLAGS}")
endif()

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
    if (${option_name} AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Applications/${target_dir}/target_libraries.cmake")
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

  include(mitkSetupBlueBerry)
endif()

#-----------------------------------------------------------------------------
# Set C/CXX Flags for MITK code
#-----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MITK_CXX_FLAGS}")
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

if(MITK_USE_QT AND QT4_FOUND)
  add_subdirectory(CoreUI/Qmitk)
endif()

add_subdirectory(Modules)

if(MITK_USE_BLUEBERRY)

  find_package(BlueBerry REQUIRED)

  set(MITK_DEFAULT_SUBPROJECTS MITK-Plugins)

  include("${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/Bundles/PluginList.cmake")
  set(mitk_core_plugins_fullpath )
  foreach(core_plugin ${MITK_CORE_PLUGINS})
    list(APPEND mitk_core_plugins_fullpath CoreUI/Bundles/${core_plugin})
  endforeach()

  if(BUILD_TESTING)
    include(berryTestingHelpers)

    include("${CMAKE_CURRENT_SOURCE_DIR}/CoreUI/BundleTesting/PluginList.cmake")
    set(mitk_core_test_plugins_fullpath )
    foreach(core_test_plugin ${MITK_CORE_TEST_PLUGINS})
    #  list(APPEND mitk_core_test_plugins_fullpath CoreUI/BundleTesting/${core_test_plugin})
    endforeach()

    set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp")
    get_target_property(_is_macosx_bundle CoreApp MACOSX_BUNDLE)
    if(APPLE AND _is_macosx_bundle)
      set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CoreApp.app/Contents/MacOS/CoreApp")
    endif()

    set(BLUEBERRY_TEST_APP_ID "org.mitk.qt.application")
  endif()

  include("${CMAKE_CURRENT_SOURCE_DIR}/Modules/Bundles/PluginList.cmake")
  set(mitk_ext_plugins_fullpath )
  foreach(ext_plugin ${MITK_EXT_PLUGINS})
    list(APPEND mitk_ext_plugins_fullpath Modules/Bundles/${ext_plugin})
  endforeach()

  if(EXISTS ${MITK_PRIVATE_MODULES}/PluginList.cmake)
    include(${MITK_PRIVATE_MODULES}/PluginList.cmake)

    foreach(ext_plugin ${MITK_PRIVATE_PLUGINS})
      list(APPEND mitk_ext_plugins_fullpath ${MITK_PRIVATE_MODULES}/${ext_plugin})
    endforeach()
  endif() 

  # Specify which plug-ins belong to this project
  MACRO(GetMyTargetLibraries all_target_libraries varname)
    SET(re_ctkplugin_mitk "^org_mitk_[a-zA-Z0-9_]+$")
    SET(re_ctkplugin_bb "^org_blueberry_[a-zA-Z0-9_]+$")
    SET(_tmp_list)
    LIST(APPEND _tmp_list ${all_target_libraries})
    ctkMacroListFilter(_tmp_list re_ctkplugin_mitk re_ctkplugin_bb OUTPUT_VARIABLE ${varname})
  ENDMACRO()

  # Add the BlueBerry plugins to the set of core plugins
  set(MITK_CORE_ENABLED_PLUGINS ${${CMAKE_PROJECT_NAME}_PLUGIN_LIBRARIES})

  # Get infos about application directories and build options
  include("${CMAKE_CURRENT_SOURCE_DIR}/Applications/AppList.cmake")
  set(mitk_apps_fullpath )
  foreach(mitk_app ${MITK_APPS})
    list(APPEND mitk_apps_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/Applications/${mitk_app}")
  endforeach()

  ctkMacroSetupPlugins(${mitk_core_plugins_fullpath} ${mitk_core_test_plugins_fullpath} ${mitk_ext_plugins_fullpath}
                       BUILD_OPTION_PREFIX MITK_BUILD_
                       APPS ${mitk_apps_fullpath}
                       BUILD_ALL ${MITK_BUILD_ALL_PLUGINS}
                       COMPACT_OPTIONS)

  ctkFunctionExtractPluginTargets("${mitk_core_plugins_fullpath}" ON _coreui_plugins)
  list(APPEND MITK_CORE_ENABLED_PLUGINS ${_coreui_plugins})

  set(MITK_PLUGIN_USE_FILE "${MITK_BINARY_DIR}/MitkPluginUseFile.cmake")
  if(${PROJECT_NAME}_PLUGIN_LIBRARIES)
    ctkFunctionGeneratePluginUseFile(${MITK_PLUGIN_USE_FILE})
  else()
    file(REMOVE ${MITK_PLUGIN_USE_FILE})
    set(MITK_PLUGIN_USE_FILE )
  endif()

  # add legacy BlueBerry bundles
  add_subdirectory(Modules/Bundles)
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
include(mitkSetupCPack)

if(MITK_BUILD_org.mitk.gui.qt.application)
  list(APPEND CPACK_CREATE_DESKTOP_LINKS "CoreApp")
endif()
if(MITK_BUILD_org.mitk.gui.qt.extapplication)
  list(APPEND CPACK_CREATE_DESKTOP_LINKS "ExtApp")
endif()


configure_file(${MITK_SOURCE_DIR}/MITKCPackOptions.cmake.in
               ${MITK_BINARY_DIR}/MITKCPackOptions.cmake @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${MITK_BINARY_DIR}/MITKCPackOptions.cmake")

# include CPack model once all variables are set
include(CPack)

# Additional installation rules
include(mitkInstallRules)

#-----------------------------------------------------------------------------
# Last configuration steps
#-----------------------------------------------------------------------------

set(MITK_EXPORTS_FILE "${MITK_BINARY_DIR}/MitkExports.cmake")
file(REMOVE ${MITK_EXPORTS_FILE})

if(MITK_USE_BLUEBERRY)
  # This is for installation support of external projects depending on
  # MITK plugins. The export file should not be used for linking to MITK
  # libraries without using LINK_DIRECTORIES, since the exports are incomplete
  # yet(depending libraries are not exported).
  if(MITK_PLUGIN_LIBRARIES)
    export(TARGETS ${MITK_PLUGIN_LIBRARIES}
           APPEND
           FILE ${MITK_EXPORTS_FILE})
  endif()
endif()

configure_file(${MITK_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in
               ${MITK_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
configure_file(${MITK_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in
               ${MITK_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
configure_file(${MITK_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in
               ${MITK_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

configure_file(mitkVersion.h.in ${MITK_BINARY_DIR}/mitkVersion.h)
configure_file(mitkConfig.h.in ${MITK_BINARY_DIR}/mitkConfig.h)

set(VECMATH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/vecmath)
set(IPFUNC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ipFunc)
set(UTILITIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Utilities)

file(GLOB _MODULES_CONF_FILES RELATIVE ${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME} ${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME}/*.cmake)
SET(MITK_MODULE_NAMES)
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

