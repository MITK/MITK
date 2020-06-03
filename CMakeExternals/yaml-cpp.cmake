#-----------------------------------------------------------------------------
# yaml-cpp
#-----------------------------------------------------------------------------

set(proj yaml-cpp)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

message(STATUS "yaml-cpp: ${YAML_CPP_DIR} , ${yaml-cpp_DIR} , ${YAML_CPP_INCLUDE_DIR} , ${YAML_CPP_LIBRARIES} .")

if(NOT DEFINED yaml-cpp_DIR OR NOT EXISTS ${yaml-cpp_DIR})
  find_package(yaml-cpp)
  message(STATUS "yaml-cpp2: ${YAML_CPP_DIR} , ${yaml-cpp_DIR} , ${YAML_CPP_INCLUDE_DIR} , ${YAML_CPP_LIBRARIES} .")
endif()

if(NOT DEFINED yaml-cpp_DIR OR NOT EXISTS ${yaml-cpp_DIR})
      # add_library(GLUT SHARED IMPORTED)
      # set_property(TARGET GLUT PROPERTY IMPORTED_LOCATION ${GLUT_LIBRARY})
  #mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  set(additional_cmake_args )

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    GIT_REPOSITORY "git@github.com:samsmu/yaml-cpp.git"
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

  message(STATUS "yaml-cpp3: ${YAML_CPP_DIR} , ${yaml-cpp_DIR} , ${YAML_CPP_INCLUDE_DIR} , ${YAML_CPP_LIBRARIES} .")

  set(${proj}_DIR ${ep_prefix})
  mitkFunctionInstallExternalCMakeProject(${proj})

  message(STATUS "yaml-cpp4: ${YAML_CPP_DIR} , ${yaml-cpp_DIR} , ${YAML_CPP_INCLUDE_DIR} , ${YAML_CPP_LIBRARIES} .")
else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  message(STATUS "yaml-cpp found: ${YAML_CPP_DIR} , ${yaml-cpp_DIR} , ${YAML_CPP_INCLUDE_DIR} , ${YAML_CPP_LIBRARIES} .")

endif()
