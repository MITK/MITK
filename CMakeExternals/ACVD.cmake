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

  set(patch_file ACVD-vtk6_2d8f5ea5.patch)
  if(MITK_USE_Qt5)
    set(patch_file ACVD-vtk6_2d8f5ea5_qt5.patch)
  endif()

  if(NOT DEFINED ACVD_DIR)
    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      BINARY_DIR ${proj}-build
      PREFIX ${proj}-cmake
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/ACVD-vtk6_2d8f5ea5.tar.gz
      URL_MD5 ecc97728a86798b35c20eef964b094c9
      PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_LIST_DIR}/${patch_file}
      INSTALL_COMMAND ""
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${ep_common_args}
        -DUSE_MULTITHREADING:BOOL=ON
        -DVTK_DIR:PATH=${VTK_DIR}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(ACVD_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
