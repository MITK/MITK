#-----------------------------------------------------------------------------
# Configure the CppMicroServices build
#-----------------------------------------------------------------------------

set(CppMicroServices_DIR_default "${CMAKE_CURRENT_BINARY_DIR}/CppMicroServices")
if(NOT CppMicroServices_DIR)
  set(CppMicroServices_DIR ${CppMicroServices_DIR_default} CACHE PATH "Path to the CppMicroServices library" FORCE)
endif()
mark_as_advanced(CppMicroServices_DIR)

if ("${CppMicroServices_DIR}" STREQUAL "${CppMicroServices_DIR_default}")

  set(US_ENABLE_AUTOLOADING_SUPPORT ON)
  set(US_ENABLE_THREADING_SUPPORT ON)

  # Don't create a "doc" target and don't install the documentation files
  set(US_NO_DOCUMENTATION ON)

  # Don't install anything for now (we use the CMake BundleUtilities script)
  set(US_NO_INSTALL 1)

  if(BUILD_TESTING)
    set(US_BUILD_TESTING ON)
  endif()

  mitkFunctionCheckCompilerFlags("-std=c++11" US_CXX_FLAGS)
  add_subdirectory(CppMicroServices)
endif()

#-----------------------------------------------------------------------------
# Configure standard MITK modules
#-----------------------------------------------------------------------------

set(MITK_DEFAULT_SUBPROJECTS MITK-Modules)

include(ModuleList.cmake)
mitkFunctionWhitelistModules(MITK mitk_modules)
