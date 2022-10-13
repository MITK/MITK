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


    set(eigen_optional_cache_args )
    if(NOT MITK_USE_CUDA)
      list(APPEND eigen_optional_cache_args
           -DCUDA_USE_STATIC_CUDA_RUNTIME:BOOL=OFF
           -DCUDA_TOOLKIT_ROOT_DIR:PATH=""
      )
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/eigen-3.4.0.tar.gz
      URL_MD5 4c527a9171d71a72a9d4186e65bea559
      CMAKE_GENERATOR ${gen}
      CMAKE_GENERATOR_PLATFORM ${gen_platform}
      CMAKE_ARGS
        ${ep_common_args}
        ${eigen_optional_cache_args}
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
