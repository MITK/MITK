#-----------------------------------------------------------------------------
# Eigen
#-----------------------------------------------------------------------------

if(MITK_USE_Eigen)

  # Sanity checks
  if(DEFINED Eigen_DIR AND NOT EXISTS ${Eigen_DIR})
    message(FATAL_ERROR "Eigen_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Eigen)
  set(proj_DEPENDENCIES )
  set(Eigen_DEPENDS ${proj})

  if(NOT DEFINED Eigen_DIR)

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/eigen-eigen-07105f7124f9.tar.bz2
      URL_MD5 9e3bfaaab3db18253cfd87ea697b3ab1
      PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Eigen.patch
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        -DBUILD_TESTING:BOOL=ON
        -DEIGEN_BUILD_PKGCONFIG:BOOL=OFF
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
    )

    set(Eigen_DIR ${ep_prefix})
    mitkFunctionInstallExternalCMakeProject(${proj})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
