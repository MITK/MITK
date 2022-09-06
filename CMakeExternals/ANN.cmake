#-----------------------------------------------------------------------------
# ANN
#-----------------------------------------------------------------------------

if(MITK_USE_ANN)

  # Sanity checks
  if(DEFINED ANN_DIR AND NOT EXISTS ${ANN_DIR})
    message(FATAL_ERROR "ANN_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj ANN)
  set(proj_DEPENDENCIES )
  set(ANN_DEPENDS ${proj})

  if(NOT DEFINED ANN_DIR)

    set(additional_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
       LIST_SEPARATOR ${sep}
       GIT_REPOSITORY https://github.com/MITK/ANN.git
       GIT_TAG v1.1.2-patched
       CMAKE_GENERATOR ${gen}
       CMAKE_GENERATOR_PLATFORM ${gen_platform}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(ANN_DIR ${ep_prefix}/lib/cmake/ANN)
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
