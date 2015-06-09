#-----------------------------------------------------------------------------
# OpenIGTLink
#-----------------------------------------------------------------------------
if(MITK_USE_OpenIGTLink)

  # Sanity checks
  if(DEFINED OpenIGTLink_DIR AND NOT EXISTS ${OpenIGTLink_DIR})
    message(FATAL_ERROR "OpenIGTLink_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj OpenIGTLink)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  if(NOT DEFINED OpenIGTLink_DIR)

    set(additional_cmake_args )
    if(MINGW)
      set(additional_cmake_args
          -DCMAKE_USE_WIN32_THREADS:BOOL=ON
          -DCMAKE_USE_PTHREADS:BOOL=OFF)
    endif()

    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/OpenIGTLink-54df50de.tar.gz
       URL_MD5 b9fd8351b059f4ec615f2dfd74ab2458
       PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/OpenIGTLink-54df50de.patch
       CMAKE_GENERATOR ${gen}
       CMAKE_ARGS
         ${ep_common_args}
         ${additional_cmake_args}
         -DBUILD_EXAMPLES:BOOL=OFF
         -DOpenIGTLink_PROTOCOL_VERSION_2:BOOL=ON
         -DOpenIGTLink_INSTALL_LIB_DIR:STRING=lib
         -DOpenIGTLink_INSTALL_PACKAGE_DIR:STRING=lib/cmake/OpenIGTLink
         -DOpenIGTLink_INSTALL_NO_DOCUMENTATION:BOOL=ON
       CMAKE_CACHE_ARGS
         ${ep_common_cache_args}
       CMAKE_CACHE_DEFAULT_ARGS
         ${ep_common_cache_default_args}
       DEPENDS ${proj_DEPENDENCIES}
      )

    set(OpenIGTLink_DIR "${ep_prefix}/lib/cmake/OpenIGTLink")

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif()
