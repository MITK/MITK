#-----------------------------------------------------------------------------
# nlohmann_json
#-----------------------------------------------------------------------------

if(MITK_USE_nlohmann_json)
  # Sanity checks
  if(DEFINED nlohmann_json_DIR AND NOT EXISTS "${nlohmann_json_DIR}")
    message(FATAL_ERROR "nlohmann_json_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj nlohmann_json)
  set(proj_DEPENDENCIES )
  set(nlohmann_json_DEPENDS ${proj})

  if(NOT DEFINED nlohmann_json_DIR)

    set(additional_args )

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.10.5
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DJSON_BuildTests:BOOL=OFF
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(nlohmann_json_DIR "${ep_prefix}")
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
