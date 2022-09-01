#-----------------------------------------------------------------------------
# PCRE (Perl Compatible Regular Expressions)
#-----------------------------------------------------------------------------

if(MITK_USE_PCRE2)
  if(DEFINED PCRE2_DIR AND NOT EXISTS "${PCRE2_DIR}")
    message(FATAL_ERROR "PCRE_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj PCRE2)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  if(NOT DEFINED ${proj}_DIR)

    set(additional_args )

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/PCRE2Project/pcre2.git
      GIT_TAG pcre2-10.40
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DPCRE2_BUILD_PCREGREP:BOOL=OFF
        -DPCRE2_BUILD_TESTS:BOOL=OFF
        -DPCRE2_STATIC_PIC:BOOL=ON
        -DPCRE2_SUPPORT_JIT:BOOL=ON
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(${proj}_DIR "${ep_prefix}")
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
