#-----------------------------------------------------------------------------
# ACVD
#-----------------------------------------------------------------------------

if(MITK_USE_ACVD)
  # Sanity checks
  if(DEFINED ACVD_DIR AND NOT EXISTS ${ACVD_DIR})
    message(FATAL_ERROR "ACVD_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj ACVD)
  set(proj_DEPENDENCIES VTK)
  set(ACVD_DEPENDS ${proj})

  if(NOT DEFINED ACVD_DIR)

    set(additional_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/valette/ACVD.git
      GIT_TAG e583e278
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_args}
        -DUSE_MULTITHREADING:BOOL=ON
        -DBUILD_EXAMPLES:BOOL=OFF
        -DQt5_DIR:PATH=${Qt5_DIR}
        -DVTK_DIR:PATH=${VTK_DIR}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(ACVD_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
