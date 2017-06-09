#-----------------------------------------------------------------------------
# Gflags
#-----------------------------------------------------------------------------

if(MITK_USE_Gflags)

  # Sanity checks
  if(DEFINED Gflags_DIR AND NOT EXISTS ${Gflags_DIR})
    message(FATAL_ERROR "Gflags_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Gflags)
  set(proj_DEPENDENCIES )
  set(Gflags_DEPENDS ${proj})

  if(WIN32)
    set(COPY_COMMAND copy)
  else()
    set(COPY_COMMAND cp)
  endif()
  
  if(NOT DEFINED Gflags_DIR)
      
    set(revision_tag v2.2.0)
    ExternalProject_Add(${proj}
       GIT_REPOSITORY https://github.com/gflags/gflags.git
       GIT_TAG ${revision_tag}
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

    set(Gflags_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_Gflags)
