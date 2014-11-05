#-----------------------------------------------------------------------------
# Configure the CppMicroServices build
#-----------------------------------------------------------------------------

set(US_ENABLE_AUTOLOADING_SUPPORT ON)
set(US_ENABLE_THREADING_SUPPORT ON)

# Don't create a "doc" target and don't install the documentation files
set(US_NO_DOCUMENTATION ON)

# Don't install anything for now (we use the CMake BundleUtilities script)
set(US_NO_INSTALL 1)

if(BUILD_TESTING)
  set(US_BUILD_TESTING ON)
endif()

set(CppMicroServices_DIR_default "${CMAKE_CURRENT_BINARY_DIR}/CppMicroServices")
if(NOT CppMicroServices_DIR)
  set(CppMicroServices_DIR ${CppMicroServices_DIR_default} CACHE PATH "Path to the CppMicroServices library" FORCE)
endif()
mark_as_advanced(CppMicroServices_DIR)

if ("${CppMicroServices_DIR}" STREQUAL "${CppMicroServices_DIR_default}")
  add_subdirectory(CppMicroServices)
endif()

# create a custom module conf file for CppMicroServices
function(_generate_cppmicroservices_conf)
  set(MITK_MODULE_NAME_PREFIX "")
  set(MODULE_IS_ENABLED 1)
  set(MODULE_NAME CppMicroServices)
  set(MODULE_TARGET ${MODULE_NAME})
  set(MODULE_EXTRA_CMAKE_CODE "find_package(CppMicroServices NO_MODULE REQUIRED)")
  set(MODULE_INCLUDE_DIRS "\${CppMicroServices_INCLUDE_DIRS}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "\${US_RUNTIME_LIBRARY_DIRS}")

  set(CppMicroServices_CONFIG_FILE "${CMAKE_BINARY_DIR}/${MODULES_CONF_DIRNAME}/CppMicroServicesConfig.cmake" CACHE INTERNAL "Path to module config" FORCE)
  configure_file(${MITK_SOURCE_DIR}/CMake/moduleConf.cmake.in ${CppMicroServices_CONFIG_FILE} @ONLY)
endfunction()

_generate_cppmicroservices_conf()

#-----------------------------------------------------------------------------
# Add the MITK Core library
#-----------------------------------------------------------------------------

set(MITK_DEFAULT_SUBPROJECTS MITK-Core)

add_subdirectory(Code)
