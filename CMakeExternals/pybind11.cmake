#-----------------------------------------------------------------------------
# pybind11
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED pybind11_DIR AND NOT EXISTS "${pybind11_DIR}")
  message(FATAL_ERROR "pybind11_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj pybind11)
set(proj_DEPENDENCIES )
set(pybind11_DIR_DEPENDS ${proj})

if(NOT DEFINED pybind11_DIR)

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG f5fbe867d2d26e4a0a9177a51f6e568868ad3dc8 # v3.0.1
    CMAKE_GENERATOR ${gen}
    CMAKE_GENERATOR_PLATFORM ${gen_platform}
    CMAKE_ARGS
      ${ep_common_args}
    CMAKE_CACHE_ARGS
      ${ep_common_cache_args}
      -DPYBIND11_NOPYTHON:BOOL=ON
      -DPYBIND11_TEST:BOOL=OFF
    CMAKE_CACHE_DEFAULT_ARGS
      ${ep_common_cache_default_args}
    DEPENDS ${proj_DEPENDENCIES}
  )

  set(pybind11_DIR "${ep_prefix}/share/cmake/pybind11")
  mitkFunctionInstallExternalCMakeProject(${proj})

else()
  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()
