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
       GIT_REPOSITORY https://github.com/HDFGroup/hdf5.git
       GIT_TAG 8b5cac6bc498546efa5639f99bb7dbbc1a2d5d90 # hdf5-1_14_3 (2023-10-28)
       CMAKE_GENERATOR ${gen}
       CMAKE_GENERATOR_PLATFORM ${gen_platform}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
         -DBUILD_TESTING:BOOL=OFF
         -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
         -DDEFAULT_API_VERSION:STRING=v18
         -DHDF5_BUILD_CPP_LIB:BOOL=ON
         -DHDF5_BUILD_EXAMPLES:BOOL=OFF
         -DHDF5_BUILD_HL_LIB:BOOL=ON
         -DHDF5_DISABLE_COMPILER_WARNINGS:BOOL=ON
         -DHDF5_ENABLE_ALL_WARNINGS:BOOL=OFF
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    ExternalProject_Get_Property(${proj} install_dir)
    if(WIN32)
      set(HDF5_DIR ${install_dir}/cmake/)
    else()
      set(HDF5_DIR ${install_dir}/share/cmake)
    endif()

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_HDF5)
