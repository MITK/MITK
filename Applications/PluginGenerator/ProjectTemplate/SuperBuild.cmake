
#-----------------------------------------------------------------------------
# ExternalProjects
#-----------------------------------------------------------------------------

set(external_projects
  MITK
  )

set(EXTERNAL_MITK_DIR "${MITK_DIR}" CACHE PATH "Path to MITK build directory")
mark_as_advanced(EXTERNAL_MITK_DIR)
if(EXTERNAL_MITK_DIR)
  set(MITK_DIR ${EXTERNAL_MITK_DIR})
endif()

# Look for git early on, if needed
if(NOT MITK_DIR AND MITK_USE_CTK AND NOT CTK_DIR)
  find_package(Git REQUIRED)
endif()

#-----------------------------------------------------------------------------
# External project settings
#-----------------------------------------------------------------------------

include(ExternalProject)

set(ep_base "${CMAKE_BINARY_DIR}/CMakeExternals")
set_property(DIRECTORY PROPERTY EP_BASE ${ep_base})

set(ep_install_dir "${CMAKE_BINARY_DIR}/CMakeExternals/Install")
set(ep_suffix "-cmake")
set(ep_build_shared_libs ON)
set(ep_build_testing OFF)

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

# Use this value where semi-colons are needed in ep_add args:
set(sep "^^")

##

if(MSVC_VERSION)
  set(ep_common_C_FLAGS "${CMAKE_C_FLAGS} /bigobj /MP")
  set(ep_common_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP")
endif()

set(ep_common_args
  -DBUILD_TESTING:BOOL=${ep_build_testing}
  -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
  -DBUILD_SHARED_LIBS:BOOL=${ep_build_shared_libs}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  # debug flags
  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
  # release flags
  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
  # relwithdebinfo
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
  -DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_C_FLAGS_RELWITHDEBINFO}
  # link flags
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}
)

set(ep_common_cache_args
)

set(ep_common_cache_default_args
  "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}"
  "-DCMAKE_INCLUDE_PATH:PATH=${CMAKE_INCLUDE_PATH}"
  "-DCMAKE_LIBRARY_PATH:PATH=${CMAKE_LIBRARY_PATH}"
)

# Include external projects
foreach(p ${external_projects})
  include(CMakeExternals/${p}.cmake)
endforeach()

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#-----------------------------------------------------------------------------

set(my_cmake_boolean_args
  WITH_COVERAGE
  BUILD_TESTING
  ${MY_PROJECT_NAME}_BUILD_ALL_PLUGINS
  )

#-----------------------------------------------------------------------------
# Create the final variable containing superbuild boolean args
#-----------------------------------------------------------------------------

set(my_superbuild_boolean_args)
foreach(my_cmake_arg ${my_cmake_boolean_args})
  list(APPEND my_superbuild_boolean_args -D${my_cmake_arg}:BOOL=${${my_cmake_arg}})
endforeach()

#-----------------------------------------------------------------------------
# Project Utilities
#-----------------------------------------------------------------------------

set(proj ${MY_PROJECT_NAME}-Utilities)
ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    # Mandatory dependencies
    ${MITK_DEPENDS}
    # Optional dependencies
)

#-----------------------------------------------------------------------------
# Additional Project CXX/C Flags
#-----------------------------------------------------------------------------

set(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_RELEASE "" CACHE STRING "Additional Release C Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_DEBUG "" CACHE STRING "Additional Debug C Flags for ${MY_PROJECT_NAME}")
mark_as_advanced(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_DEBUG ${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_RELEASE)

set(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_RELEASE "" CACHE STRING "Additional Release CXX Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_DEBUG "" CACHE STRING "Additional Debug CXX Flags for ${MY_PROJECT_NAME}")
mark_as_advanced(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_DEBUG ${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_RELEASE)

set(${MY_PROJECT_NAME}_ADDITIONAL_EXE_LINKER_FLAGS "" CACHE STRING "Additional exe linker flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_SHARED_LINKER_FLAGS "" CACHE STRING "Additional shared linker flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_MODULE_LINKER_FLAGS "" CACHE STRING "Additional module linker flags for ${MY_PROJECT_NAME}")
mark_as_advanced(${MY_PROJECT_NAME}_ADDITIONAL_EXE_LINKER_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_SHARED_LINKER_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_MODULE_LINKER_FLAGS)

#-----------------------------------------------------------------------------
# Project Configure
#-----------------------------------------------------------------------------

set(proj ${MY_PROJECT_NAME}-Configure)

ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  CMAKE_CACHE_ARGS
    # --------------- Build options ----------------
    -DBUILD_TESTING:BOOL=${ep_build_testing}
    -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
    -DBUILD_SHARED_LIBS:BOOL=${ep_build_shared_libs}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}"
    "-DCMAKE_INCLUDE_PATH:PATH=${CMAKE_INCLUDE_PATH}"
    "-DCMAKE_LIBRARY_PATH:PATH=${CMAKE_LIBRARY_PATH}"
    # --------------- Compile options ----------------
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS}"
    "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS}"
    # debug flags
    "-DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG} ${${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_DEBUG}"
    "-DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG} ${${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_DEBUG}"
    # release flags
    "-DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE} ${${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_RELEASE}"
    "-DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE} ${${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_RELEASE}"
    # relwithdebinfo
    -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
    -DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_C_FLAGS_RELWITHDEBINFO}
    # link flags
    "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_EXE_LINKER_FLAGS}"
    "-DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_SHARED_LINKER_FLAGS}"
    "-DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_MODULE_LINKER_FLAGS}"
    # ------------- Boolean build options --------------
    ${my_superbuild_boolean_args}
    -D${MY_PROJECT_NAME}_USE_SUPERBUILD:BOOL=OFF
    -D${MY_PROJECT_NAME}_CONFIGURED_VIA_SUPERBUILD:BOOL=ON
    -DCTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
    # ----------------- Miscellaneous ---------------
    -D${MY_PROJECT_NAME}_SUPERBUILD_BINARY_DIR:PATH=${PROJECT_BINARY_DIR}
    -DMITK_DIR:PATH=${MITK_DIR}
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}

  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/${MY_PROJECT_NAME}-build
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${MY_PROJECT_NAME}-Utilities
  )


#-----------------------------------------------------------------------------
# Project
#-----------------------------------------------------------------------------

if(CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  set(_build_cmd "$(MAKE)")
else()
  set(_build_cmd ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build --config ${CMAKE_CFG_INTDIR})
endif()

# The variable SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET should be set when submitting to a dashboard
if(NOT DEFINED SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET)
  set(_target_all_option "ALL")
else()
  set(_target_all_option "")
endif()

add_custom_target(${MY_PROJECT_NAME}-build ${_target_all_option}
  COMMAND ${_build_cmd}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build
  DEPENDS ${MY_PROJECT_NAME}-Configure
  )

#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of the project itself
#-----------------------------------------------------------------------------

add_custom_target(${MY_PROJECT_NAME}
  COMMAND ${_build_cmd}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build
)

