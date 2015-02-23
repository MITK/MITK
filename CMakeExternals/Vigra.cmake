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

  if(NOT DEFINED Vigra_DIR)

    set(patch_cmd ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${CMAKE_CURRENT_LIST_DIR}/EmptyFileForPatching.dummy -DHDF5_DIR:PATH=${HDF5_DIR} -P ${CMAKE_CURRENT_LIST_DIR}/PatchVigra-1.10.0.cmake)

    ExternalProject_Add(${proj}
       SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
       BINARY_DIR ${proj}-build
       INSTALL_DIR ${proj}-install
       PREFIX ${proj}-cmake
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/vigra-1.10.0-src.tar.gz
       URL_MD5 4f963f0be4fcb8b06271c2aa40baa9be
       PATCH_COMMAND ${patch_cmd}
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         -DAUTOEXEC_TESTS:BOOL=OFF
         -DWITH_VIGRANUMPY:BOOL=OFF
         -DHDF5_DIR:PATH=${HDF5_DIR}
         -DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=TRUE
         -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(Vigra_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-install)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif(MITK_USE_Vigra)
