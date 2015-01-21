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
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/ACVD-vtk6_3d5ae388.tar.gz
      URL_MD5 5d47000a6ee3be0ec8ac7dedc04896c0
      PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ACVD-vtk6_3d5ae388.patch
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DUSE_MULTITHREADING:BOOL=ON
        -DBUILD_EXAMPLES:BOOL=OFF
        -DVTK_DIR:PATH=${VTK_DIR}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(ACVD_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
