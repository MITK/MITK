#-----------------------------------------------------------------------------
# Configure the CppMicroServices build
#-----------------------------------------------------------------------------

set(US_ENABLE_AUTOLOADING_SUPPORT ON)
set(US_ENABLE_THREADING_SUPPORT ON)
set(US_IS_EMBEDDED OFF)

# Don't create a "doc" target and don't install the documentation files
set(US_NO_DOCUMENTATION ON)

# Don't use an install component for SDK artifacts
set(US_SDK_INSTALL_COMPONENT "")

if(BUILD_TESTING)
  set(US_BUILD_TESTING ON)
endif()

add_subdirectory(CppMicroServices)
set(CppMicroServices_DIR ${CMAKE_CURRENT_BINARY_DIR}/CppMicroServices CACHE PATH "Path to the CppMicroServices library")
mark_as_advanced(CppMicroServices_DIR)

# create a custom module conf file for CppMicroServices
function(_generate_cppmicroservices_conf)
  set(MODULE_IS_ENABLED 1)
  set(MODULE_NAME CppMicroServices)
  set(MODULE_PROVIDES ${MODULE_NAME})
  set(MODULE_EXTRA_CMAKE_CODE "find_package(CppMicroServices NO_MODULE REQUIRED)")
  set(MODULE_INCLUDE_DIRS "\${CppMicroServices_INCLUDE_DIRS}")

  set(CppMicroServices_CONFIG_FILE "${CMAKE_BINARY_DIR}/${MODULES_CONF_DIRNAME}/CppMicroServicesConfig.cmake" CACHE INTERNAL "Path to module config" FORCE)
  configure_file(${MITK_SOURCE_DIR}/CMake/moduleConf.cmake.in ${CppMicroServices_CONFIG_FILE} @ONLY)
endfunction()

_generate_cppmicroservices_conf()

#-----------------------------------------------------------------------------
# Add the MITK Core library
#-----------------------------------------------------------------------------

set(MITK_DEFAULT_SUBPROJECTS MITK-Core)

add_subdirectory(Code)
