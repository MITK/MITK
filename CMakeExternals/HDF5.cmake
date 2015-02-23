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

         set(HDF5_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)

    if(APPLE)
      set(APPLE_HDF5_CMAKE_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${proj}-cmake/ChangeHDF5LibsInstallNameForMac.cmake)
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeExternals/ChangeHDF5LibsInstallNameForMac.cmake.in ${APPLE_HDF5_CMAKE_SCRIPT} @ONLY)
    endif()

    # We might build static libs with  -DBUILD_SHARED_LIBS=0 but this conflicts with
    # the in ITK integrated version! So we need to go the way with dynamic libs. Too
    # bad :( This would be fixed by using an external HDF-Installation with ITK/VTK

    ExternalProject_Add(${proj}
       SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
       BINARY_DIR ${proj}-build
       PREFIX ${proj}-cmake
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/hdf5-1.8.12.tar.gz
       URL_MD5 d804802feb99b87fc668a90e6fa34411
       INSTALL_DIR ${proj}-install
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         -DHDF5_BUILD_HL_LIB:BOOL=ON
         -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       DEPENDS ${proj_DEPENDENCIES}
      )

    if(APPLE)
          ExternalProject_Add_Step(${proj} sitk_create_userbase_step
            COMMAND ${CMAKE_COMMAND} -P ${APPLE_HDF5_CMAKE_SCRIPT}
        DEPENDEES install
        WORKING_DIRECTORY ${HDF5_DIR}
      )
    endif()

    set(HDF5_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_HDF5)
