#-----------------------------------------------------------------------------
# Boost
#-----------------------------------------------------------------------------

if(MITK_USE_Boost)

  # Sanity checks
  if(DEFINED BOOST_ROOT AND NOT EXISTS ${BOOST_ROOT})
    message(FATAL_ERROR "BOOST_ROOT variable is defined but corresponds to non-existing directory")
  endif()

  set(proj Boost)
  set(proj_DEPENDENCIES )
  set(Boost_DEPENDS ${proj})

  if(NOT DEFINED BOOST_ROOT AND NOT MITK_USE_SYSTEM_Boost)

    set(_boost_libs )

    if(MITK_USE_Boost_LIBRARIES)
      # We need binary boost libraries
      foreach(_boost_lib ${MITK_USE_Boost_LIBRARIES})
        set(_boost_libs ${_boost_libs} --with-${_boost_lib})
      endforeach()

      if(WIN32)
        set(_boost_variant "")
        set(_shell_extension .bat)
      else()
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
          set(_boost_variant "variant=debug")
        else()
          set(_boost_variant "variant=release")
        endif()
        set(_shell_extension .sh)
      endif()

      set(_boost_cfg_cmd ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/bootstrap${_shell_extension})
      set(_boost_build_cmd ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/bjam --build-dir=${CMAKE_CURRENT_BINARY_DIR}/${proj}-build --prefix=${CMAKE_CURRENT_BINARY_DIR}/${proj}-install ${_boost_variant} ${_boost_libs} link=shared,static threading=multi runtime-link=shared -q install)
    else()
      set(_boost_cfg_cmd )
      set(_boost_build_cmd )
    endif()

    ExternalProject_Add(${proj}
      SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}-src
      # Boost needs in-source builds
      BINARY_DIR ${proj}-src
      PREFIX ${proj}-cmake
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/boost_1_45_0.tar.bz2
      URL_MD5 d405c606354789d0426bc07bea617e58
      INSTALL_DIR ${proj}-install
      CONFIGURE_COMMAND "${_boost_cfg_cmd}"
      BUILD_COMMAND "${_boost_build_cmd}"
      INSTALL_COMMAND ""
      DEPENDS ${proj_DEPENDENCIES}
      )

    if(MITK_USE_Boost_LIBRARIES)
      set(BOOST_ROOT "${CMAKE_CURRENT_BINARY_DIR}/${proj}-install")
    else()
      set(BOOST_ROOT "${CMAKE_CURRENT_BINARY_DIR}/${proj}-src")
    endif()

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
