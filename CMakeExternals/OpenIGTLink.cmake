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

    if(CTEST_USE_LAUNCHERS)
      set(additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    ExternalProject_Add(${proj}
       URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/OpenIGTLink-release-3.0.tar.gz
       URL_MD5 0a759655da037f6df2087dd2690d1ae2
       PATCH_COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/OpenIGTLink-92bc3d7b.patch
       CMAKE_GENERATOR ${gen}
       CMAKE_GENERATOR_PLATFORM ${gen_platform}
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
