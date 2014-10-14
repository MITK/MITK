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
    set(INSTALL_COMMAND "")

    if(MITK_USE_Boost_LIBRARIES)

      # Set the boost root to the libraries install directory
      set(BOOST_ROOT "${CMAKE_CURRENT_BINARY_DIR}/${proj}-install")

      # We need binary boost libraries
      foreach(_boost_lib ${MITK_USE_Boost_LIBRARIES})
        set(_boost_libs ${_boost_libs} --with-${_boost_lib})
      endforeach()

      if(WIN32)
        set(_boost_variant "")
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
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
          set(_boost_variant "variant=debug")
        else()
          set(_boost_variant "variant=release")
        endif()
        set(_shell_extension .sh)
      endif()

      if(APPLE)
        set(APPLE_CMAKE_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/${proj}-cmake/ChangeBoostLibsInstallNameForMac.cmake)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeExternals/ChangeBoostLibsInstallNameForMac.cmake.in ${APPLE_CMAKE_SCRIPT} @ONLY)
        set(INSTALL_COMMAND ${CMAKE_COMMAND} -P ${APPLE_CMAKE_SCRIPT})

        # Set OSX_SYSROOT
        set (APPLE_SYSROOT_FLAG)
        if (NOT ${CMAKE_OSX_SYSROOT} STREQUAL "")
          set (APPLE_SYSROOT_FLAG --sysroot=${CMAKE_OSX_SYSROOT})
        endif()

        # Set the boost build command for apple
        set(_boost_build_cmd ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/bjam ${APPLE_SYSROOT_FLAG} --builddir=${CMAKE_CURRENT_BINARY_DIR}/${proj}-build --prefix=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install
            ${_boost_toolset} ${_boost_address_model} ${_boost_variant} ${_boost_libs} link=shared,static threading=multi runtime-link=shared -q install)
      else()
        set(_boost_build_cmd ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/bjam --build-dir=${CMAKE_CURRENT_BINARY_DIR}/${proj}-build --prefix=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install ${_boost_toolset} ${_boost_address_model}
            ${_boost_variant} ${_boost_libs} link=shared,static threading=multi runtime-link=shared -q install)
      endif()

      set(_boost_cfg_cmd ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/bootstrap${_shell_extension})

    else()
      # If no libraries are specified set the boost root to the boost src directory
      set(BOOST_ROOT "${CMAKE_CURRENT_BINARY_DIR}/${proj}-src")
      set(_boost_cfg_cmd )
      set(_boost_build_cmd )
    endif()

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      # Boost needs in-source builds
      BINARY_DIR ${proj}-src
      PREFIX ${proj}-cmake
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/boost_1_56_0.tar.bz2
      URL_MD5 a744cf167b05d72335f27c88115f211d
      INSTALL_DIR ${proj}-install
      CONFIGURE_COMMAND "${_boost_cfg_cmd}"
      BUILD_COMMAND "${_boost_build_cmd}"
      INSTALL_COMMAND "${INSTALL_COMMAND}"
      DEPENDS ${proj_DEPENDENCIES}
      )

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
