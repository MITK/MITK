#-----------------------------------------------------------------------------
# VMTK
#-----------------------------------------------------------------------------

if(MITK_USE_VMTK)
  set(proj VMTK)
  set(proj_DEPENDENCIES ITK VTK)
  set(${proj}_DEPENDS ${proj})

  # Sanity checks
  if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    set(additional_cmake_args)

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_VTK_VMTK_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/vtkVmtk-7fa3faf6.tar.bz2
      URL_MD5 278725a50a18591cc8e24f8aa81a6fc5
      PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/VMTK.patch
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        ${additional_cmake_args}
        -DVTK_VMTK_BUILD_STREAMTRACER:BOOL=OFF
        -DVTK_VMTK_BUILD_TETGEN:BOOL=OFF
        -DVTK_VMTK_WRAP_PYTHON:BOOL=OFF
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(${proj}_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
