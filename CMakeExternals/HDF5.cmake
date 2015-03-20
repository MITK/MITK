#-----------------------------------------------------------------------------
# HDF5
#-----------------------------------------------------------------------------

if(MITK_USE_HDF5)

  # Sanity checks
  if(DEFINED HDF5_DIR AND NOT EXISTS ${HDF5_DIR})
    message(FATAL_ERROR "HDF5_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj HDF5)
  set(proj_DEPENDENCIES )
  set(HDF5_DEPENDS ${proj})

  if(NOT DEFINED HDF5_DIR)

    set(additional_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    # We might build static libs with  -DBUILD_SHARED_LIBS=0 but this conflicts with
    # the in ITK integrated version! So we need to go the way with dynamic libs. Too
    # bad :( This would be fixed by using an external HDF-Installation with ITK/VTK

    ExternalProject_Add(${proj}
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/hdf5-1.8.12.tar.gz
       URL_MD5 d804802feb99b87fc668a90e6fa34411
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
         -DHDF5_BUILD_HL_LIB:BOOL=ON
         -DHDF5_BUILD_CPP_LIB:BOOL=ON
         -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    ExternalProject_Get_Property(${proj} install_dir)
    if(WIN32)
      set(HDF5_DIR ${install_dir}/cmake/hdf5)
    else()
      set(HDF5_DIR ${install_dir}/share/cmake/hdf5)
    endif()

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_HDF5)
