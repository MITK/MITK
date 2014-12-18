#-----------------------------------------------------------------------------
# OpenIGTLink
#-----------------------------------------------------------------------------

if(MITK_USE_OpenIGTLink)

  # Sanity checks
  if(DEFINED OpenIGTLink_DIR AND NOT EXISTS ${OpenIGTLink_DIR})
    message(FATAL_ERROR "OpenIGTLink_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj OpenIGTLink)

  set(OPENIGTLINK_DEPENDS ${proj})

  if(NOT DEFINED OpenIGTLink_DIR)

    set(additional_cmake_args )
    if(MINGW)
      set(additional_cmake_args
          -DCMAKE_USE_WIN32_THREADS:BOOL=ON
          -DCMAKE_USE_PTHREADS:BOOL=OFF)
    endif()

    #list(APPEND additional_cmake_args
    #     -DUSE_WRAP_ITK:BOOL=OFF
    #    )

   # set(ITK_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${MITK_SOURCE_DIR}/CMakeExternals/EmptyFileForPatching.dummy -P ${MITK_SOURCE_DIR}/CMakeExternals/PatchITK-4.5.1.cmake)

    ExternalProject_Add(${proj}
       SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
       BINARY_DIR ${proj}-build
       PREFIX ${proj}-cmake
       URL /projects/OpenIGTLink-54df50de62511f8fd07a7ce5b308f1202b30933a.tar.gz
       URL_MD5 b9fd8351b059f4ec615f2dfd74ab2458
       INSTALL_COMMAND ""
       #PATCH_COMMAND ${ITK_PATCH_COMMAND}
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_cmake_args}
         -DBUILD_TESTING:BOOL=OFF
         -DBUILD_EXAMPLES:BOOL=OFF
         -DOpenIGTLink_PROTOCOL_VERSION_2:BOOL=ON
         -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
         -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
         #-DITK_USE_SYSTEM_GDCM:BOOL=ON
         #-DGDCM_DIR:PATH=${GDCM_DIR}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(OpenIGTLink_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif()
