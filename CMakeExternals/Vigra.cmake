#-----------------------------------------------------------------------------
# VIGRA
#-----------------------------------------------------------------------------

if(MITK_USE_Vigra)

  # Sanity checks
  if(DEFINED Vigra_DIR AND NOT EXISTS ${Vigra_DIR})
    message(FATAL_ERROR "Vigra_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT ${MITK_USE_HDF5})
    message(FATAL_ERROR "HDF5 is required for Vigra. Please enable it.")
  endif()

  set(proj Vigra)
  set(proj_DEPENDENCIES HDF5)
  set(Vigra_DEPENDS ${proj})

  # If a mac ports installation is present some imaging-io-libraries may interfere with the vigra build.
  # Hence, we exclude them here.
  set(mac_additional_cmake_args)
  if(APPLE)
    set(mac_additional_cmake_args -DJPEG_INCLUDE_DIR= -DJPEG_LIBRARY= -DTIFF_INCLUDE_DIR= -DTIFF_LIBRARY= -DPNG_LIBRARY_RELEASE= -DPNG_PNG_INCLUDE_DIR= )
  endif()

  if(NOT DEFINED Vigra_DIR)

    set(additional_cmake_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/vigra-1.10.0-src.tar.gz
       URL_MD5 4f963f0be4fcb8b06271c2aa40baa9be
       PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Vigra.patch
       CMAKE_GENERATOR ${gen}
       CMAKE_GENERATOR_PLATFORM ${gen_platform}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_cmake_args}
         ${mac_additional_cmake_args}
         -DAUTOEXEC_TESTS:BOOL=OFF
         -DWITH_VIGRANUMPY:BOOL=OFF
         -DHDF5_DIR:PATH=${HDF5_DIR}
         -DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=TRUE
         -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(Vigra_DIR ${ep_prefix})

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_Vigra)
