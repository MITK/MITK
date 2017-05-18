#-----------------------------------------------------------------------------
# GFLAGS
#-----------------------------------------------------------------------------

if(MITK_USE_GFLAGS)

  # Sanity checks
  if(DEFINED GFLAGS_DIR AND NOT EXISTS ${GFLAGS_DIR})
    message(FATAL_ERROR "GFLAGS_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj GFLAGS)
  set(proj_DEPENDENCIES )
  set(GFLAGS_DEPENDS ${proj})

  if(NOT DEFINED GFLAGS_DIR)

    ExternalProject_Add(${proj}
       URL /home/neher/Downloads/gflags-2.2.0.zip
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )
    
    set(GFLAGS_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_GFLAGS)
