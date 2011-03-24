cmake_minimum_required(VERSION 2.8.2)

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

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

foreach(type LIBRARY RUNTIME ARCHIVE)
  # Make sure the directory exists
  if(DEFINED MITK_CMAKE_${type}_OUTPUT_DIRECTORY
     AND NOT EXISTS ${MITK_CMAKE_${type}_OUTPUT_DIRECTORY})
    message(FATAL_ERROR "MITK_CMAKE_${type}_OUTPUT_DIRECTORY is set to a non-existing directory [${MITK_CMAKE_${type}_OUTPUT_DIRECTORY}]")
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

option(MITK_INSTALL_RPATH_RELATIVE "Use relative rpath entries for installation packages" OFF)
option(MITK_BUILD_ALL_PLUGINS "Build all MITK plugins" OFF)
option(MITK_BUILD_TUTORIAL "Build the MITK tutorial" ON)
option(MITK_USE_Boost "Use the Boost C++ library" OFF)
option(MITK_USE_GDCMIO "Use the GDCMIO class instead of ImageIO2 for DICOM" ON)
option(MITK_USE_BLUEBERRY "Build the BlueBerry platform" ON)
option(MITK_USE_CTK "EXEPERIMENTAL, superbuild only: Use CTK in MITK" OFF)
option(MITK_USE_DCMTK "EXEPERIMENTAL, superbuild only: Use DCMTK in MITK" OFF)
option(MITK_USE_OPEN_CV "Use Intel's OpenCV library" OFF)
  
mark_as_advanced(MITK_INSTALL_RPATH_RELATIVE
                 MITK_BUILD_ALL_PLUGINS
                 MITK_USE_CTK
                 MITK_USE_DCMTK
                 )

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
  )

# Configure CTestConfigSubProject.cmake that could be used by CTest scripts
configure_file(${MITK_SOURCE_DIR}/CTestConfigSubProject.cmake.in
               ${MITK_BINARY_DIR}/CTestConfigSubProject.cmake)

# Generate Project.xml file expected by the CTest driver script
mitkFunctionGenerateProjectXml(${MITK_BINARY_DIR} ${PROJECT_NAME} "${CTEST_PROJECT_SUBPROJECTS}" ${MITK_USE_SUPERBUILD})

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

include(mitkFunctionCheckCompilerFlags)
include(mitkFunctionGetGccVersion)
include(MacroParseArguments)
include(mitkFunctionSuppressWarnings) # includes several functions
include(mitkFunctionOrganizeSources)
include(mitkFunctionGetVersion)
include(mitkFunctionCreateWindowsBatchScript)
include(mitkMacroCreateModuleConf)
include(mitkMacroCreateModule)
include(mitkMacroCheckModule)
include(mitkMacroCreateModuleTests)
include(mitkMacroUseModule)
include(mitkMacroMultiplexPicType)
include(mitkMacroInstall)
include(mitkMacroInstallHelperApp)
include(mitkMacroInstallTargets)
include(mitkMacroGenerateToolsLibrary)

#-----------------------------------------------------------------------------
# Prerequesites
#-----------------------------------------------------------------------------

find_package(ITK REQUIRED)
find_package(VTK REQUIRED)

#-----------------------------------------------------------------------------
# Set MITK specific options and variables (NOT available during superbuild)
#-----------------------------------------------------------------------------

# ASK THE USER TO SHOW THE CONSOLE WINDOW FOR CoreApp and ExtApp
option(MITK_SHOW_CONSOLE_WINDOW "Use this to enable or disable the console window when starting MITK GUI Applications" ON)
mark_as_advanced(MITK_SHOW_CONSOLE_WINDOW)

# TODO: check if necessary
option(USE_ITKZLIB "Use the ITK zlib for pic compression." ON)
mark_as_advanced(USE_ITKZLIB)

#-----------------------------------------------------------------------------
# Get MITK version info
#-----------------------------------------------------------------------------

mitkFunctionGetVersion(${MITK_SOURCE_DIR} MITK)

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

set(MITK_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_C_FLAGS} ${ADDITIONAL_C_FLAGS}")
set(MITK_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VISIBILITY_CXX_FLAGS} ${COVERAGE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(cflags "-Wall -Wextra -Wpointer-arith -Winvalid-pch -Wcast-align -Wwrite-strings -D_FORTIFY_SOURCE=2")
  
  mitkFunctionCheckCompilerFlags("-fdiagnostics-show-option" cflags)
  mitkFunctionCheckCompilerFlags("-Wl,--no-undefined" cflags)
  
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
# MITK Modules
#-----------------------------------------------------------------------------

set(MITK_MODULES_CONF_DIR ${MITK_BINARY_DIR}/modulesConf CACHE INTERNAL "Modules Conf")
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
  configure_file(mitkTestingConfig.h.in ${MITK_BINARY_DIR}/mitkTestingConfig.h)
endif()

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

add_subdirectory(Utilities)

include(mitkSetupVariables)
if(MITK_USE_BLUEBERRY)
  include(mitkSetupBlueBerry)
endif()

#-----------------------------------------------------------------------------
# Set C/CXX Flags for MITK code
#-----------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS ${MITK_CXX_FLAGS})
set(CMAKE_C_FLAGS ${MITK_C_FLAGS})

if(MITK_USE_QT)
  add_definitions(-DQWT_DLL)
endif()

#-----------------------------------------------------------------------------
# Add custom targets representing CDash subprojects
#-----------------------------------------------------------------------------

foreach(subproject ${CTEST_PROJECT_SUBPROJECTS})
  if(NOT TARGET ${subproject})
    add_custom_target(${subproject})
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Add subdirectories
#-----------------------------------------------------------------------------

link_directories(${MITK_LINK_DIRECTORIES})

add_subdirectory(Core)
add_subdirectory(CoreUI)
add_subdirectory(Modules)
add_subdirectory(Applications)

#-----------------------------------------------------------------------------
# Python Wrapping
#-----------------------------------------------------------------------------

set(MITK_WRAPPING_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping)
set(MITK_WRAPPING_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Wrapping)
option(MITK_USE_PYTHON "Build cswig Python wrapper support (requires CableSwig)." OFF)
if(MITK_USE_PYTHON)
  add_subdirectory(Wrapping)
endif()

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

add_subdirectory(Documentation)

#-----------------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------------

if(MITK_INSTALL_RPATH_RELATIVE)
  set(CMAKE_INSTALL_RPATH ".")
else()
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
endif()

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
  set(enabled_plugins
    ${BLUEBERRY_ENABLED_PLUGINS}
    ${MITK_CORE_ENABLED_PLUGINS}
    ${MITK_MODULES_ENABLED_PLUGINS}
    )

  # This is for installation support of external projects depending on
  # MITK plugins. The export file should not be used for linking to MITK
  # libraries without using LINK_DIRECTORIES, since the exports are incomplete
  # yet(depending libraries are not exported).
  foreach(plugin ${enabled_plugins})
    string(REPLACE "." "_" plugin_target ${plugin})
    export(TARGETS ${plugin_target}
           APPEND
           FILE ${MITK_EXPORTS_FILE})
  endforeach()
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

configure_file(mitkConfig.h.in ${MITK_BINARY_DIR}/mitkConfig.h)
configure_file(MITKConfig.cmake.in ${MITK_BINARY_DIR}/MITKConfig.cmake @ONLY)

