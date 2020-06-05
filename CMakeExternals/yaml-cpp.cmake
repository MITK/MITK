#-----------------------------------------------------------------------------
# yaml-cpp
#-----------------------------------------------------------------------------

set(proj yaml-cpp)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

find_package(${proj} 0.6.2)

if(NOT EXISTS "${YAML_CPP_INCLUDE_DIR}" AND "${YAML_CPP_INCLUDE_DIR}" MATCHES .*/\.\./include)
  string(REPLACE /../i /i YAML_CPP_INCLUDE_DIR "${YAML_CPP_INCLUDE_DIR}")
endif()

if(NOT DEFINED ${proj}_DIR OR NOT EXISTS "${${proj}_DIR}")

  set(additional_cmake_args -DYAML_BUILD_SHARED_LIBS=ON)

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

elseif(NOT TARGET ${proj})

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
