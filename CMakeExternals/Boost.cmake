#-----------------------------------------------------------------------------
# Boost
#-----------------------------------------------------------------------------

if(MITK_USE_Boost)

  # Sanity checks
  if(DEFINED BOOST_ROOT AND NOT EXISTS ${BOOST_ROOT})
    message(FATAL_ERROR "BOOST_ROOT variable is defined but corresponds to non-existing directory")
  endif()

  string(REPLACE "^^" ";" MITK_USE_Boost_LIBRARIES "${MITK_USE_Boost_LIBRARIES}")

  set(proj Boost)
  set(proj_DEPENDENCIES )
  set(Boost_DEPENDS ${proj})

  if(NOT DEFINED BOOST_ROOT AND NOT MITK_USE_SYSTEM_Boost)

    set(_boost_libs )

    # Set the boost root to the libraries install directory
    set(BOOST_ROOT "${ep_prefix}")

    if(MITK_USE_Boost_LIBRARIES)
      string(REPLACE ";" "," _boost_libs "${MITK_USE_Boost_LIBRARIES}")
    endif()

    if(WIN32)
      set(_shell_extension .bat)
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_boost_address_model "address-model=64")
      else()
        set(_boost_address_model "address-model=32")
      endif()
      if(MSVC)
        if(MSVC_VERSION EQUAL 1400)
          set(_boost_toolset "toolset=msvc-8.0")
        elseif(MSVC_VERSION EQUAL 1500)
          set(_boost_toolset "toolset=msvc-9.0")
        elseif(MSVC_VERSION EQUAL 1600)
          set(_boost_toolset "toolset=msvc-10.0")
        elseif(MSVC_VERSION EQUAL 1700)
          set(_boost_toolset "toolset=msvc-11.0")
        endif()
      endif()
    else()
      set(_shell_extension .sh)
    endif()

    set (APPLE_SYSROOT_FLAG)
    if(APPLE)
      #set(APPLE_CMAKE_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${proj}-cmake/ChangeBoostLibsInstallNameForMac.cmake)
      #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeExternals/ChangeBoostLibsInstallNameForMac.cmake.in ${APPLE_CMAKE_SCRIPT} @ONLY)
      #set(INSTALL_COMMAND ${CMAKE_COMMAND} -P ${APPLE_CMAKE_SCRIPT})

      # Set OSX_SYSROOT
      if (NOT ${CMAKE_OSX_SYSROOT} STREQUAL "")
        set (APPLE_SYSROOT_FLAG --sysroot=${CMAKE_OSX_SYSROOT})
      endif()
    endif()

    set(_boost_variant "$<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>")
    set(_boost_link shared)
    if(NOT BUILD_SHARED_LIBS)
      set(_boost_link static)
    endif()

    set(_build_cmd ${CMAKE_COMMAND} -E chdir "<SOURCE_DIR>"
        ./b2
        ${APPLE_SYSROOT_FLAG}
        "--build-dir=<BINARY_DIR>"
        variant=${_boost_variant}
        link=${_boost_link}
        threading=multi
        runtime-link=shared
        -q
    )

    if(MITK_USE_Boost_LIBRARIES)
      set(_boost_build_cmd BUILD_COMMAND ${_build_cmd})
      set(_install_cmd ${_build_cmd} install)
    else()
      set(_boost_build_cmd BUILD_COMMAND ${CMAKE_COMMAND} -E echo "no binary libraries")
      set(_install_cmd ${CMAKE_COMMAND} -E echo "copying Boost header..."
             COMMAND ${CMAKE_COMMAND} -E copy_directory "<SOURCE_DIR>/boost" "<INSTALL_DIR>/include/boost")
    endif()

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/boost_1_56_0.tar.bz2
      URL_MD5 a744cf167b05d72335f27c88115f211d
      CONFIGURE_COMMAND ${CMAKE_COMMAND} -E chdir "<SOURCE_DIR>" ./bootstrap${_shell_extension}
        --with-toolset=${_boost_toolset}
        --with-libraries=${_boost_libs}
        "--prefix=<INSTALL_DIR>"
      ${_boost_build_cmd}
      INSTALL_COMMAND ${_install_cmd}
      DEPENDS ${proj_DEPENDENCIES}
      )

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
