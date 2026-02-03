#-----------------------------------------------------------------------------
# TBB
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED TBB_DIR AND NOT EXISTS "${TBB_DIR}")
  message(FATAL_ERROR "TBB_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj TBB)
set(proj_DEPENDENCIES )
set(TBB_DEPENDS ${proj})

if(NOT DEFINED TBB_DIR)

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    GIT_REPOSITORY https://github.com/uxlfoundation/oneTBB.git
    GIT_TAG f1862f38f83568d96e814e469ab61f88336cc595 # v2022.3.0
    CMAKE_GENERATOR ${gen}
    CMAKE_GENERATOR_PLATFORM ${gen_platform}
    CMAKE_ARGS
      ${ep_common_args}
    CMAKE_CACHE_ARGS
      ${ep_common_cache_args}
      -DTBB_TEST:BOOL=OFF
      -DTBB_STRICT:BOOL=OFF
    CMAKE_CACHE_DEFAULT_ARGS
      ${ep_common_cache_default_args}
    DEPENDS ${proj_DEPENDENCIES}
  )

  set(TBB_DIR "${ep_prefix}")
  mitkFunctionInstallExternalCMakeProject(${proj})

else()
  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()
