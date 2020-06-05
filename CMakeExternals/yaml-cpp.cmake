#-----------------------------------------------------------------------------
# yaml-cpp
#-----------------------------------------------------------------------------

set(proj yaml-cpp)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(NOT DEFINED yaml-cpp_DIR OR NOT EXISTS ${yaml-cpp_DIR})
  find_package(yaml-cpp)
endif()

if(NOT DEFINED yaml-cpp_DIR OR NOT EXISTS ${yaml-cpp_DIR})

  set(additional_cmake_args )

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    GIT_REPOSITORY "git@github.com:samsmu/yaml-cpp.git"
    GIT_TAG "yaml-cpp-0.6.3"
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      ${ep_common_args}
      ${additional_cmake_args}
    CMAKE_CACHE_ARGS
      ${ep_common_cache_args}
    CMAKE_CACHE_DEFAULT_ARGS
      ${ep_common_cache_default_args}
    DEPENDS ${proj_DEPENDENCIES}
  )

  set(${proj}_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
