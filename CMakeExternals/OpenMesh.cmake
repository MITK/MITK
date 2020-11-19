#-----------------------------------------------------------------------------
# OpenMesh
#-----------------------------------------------------------------------------

if(MITK_USE_OpenMesh)
  # Sanity checks
  if(DEFINED OpenMesh_DIR AND NOT EXISTS "${OpenMesh_DIR}")
    message(FATAL_ERROR "OpenMesh_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj OpenMesh)
  set(proj_DEPENDENCIES )
  set(OpenMesh_DEPENDS ${proj})

  if(NOT DEFINED OpenMesh_DIR)

    set(additional_args )

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/OpenMesh-8.1.tar.gz
      URL_MD5 9e1eb6feeca3882ab95f9fc97681a4da
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_args}
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
        -DBUILD_APPS:BOOL=OFF
        -DOPENMESH_BUILD_SHARED:BOOL=ON
        -DOPENMESH_DOCS:BOOL=OFF
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(OpenMesh_DIR "${ep_prefix}")
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
