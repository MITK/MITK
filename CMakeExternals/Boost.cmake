#-----------------------------------------------------------------------------
# Boost
#-----------------------------------------------------------------------------

include(mitkFunctionGetMSVCVersion)

# Sanity checks
if(DEFINED BOOST_ROOT AND NOT EXISTS ${BOOST_ROOT})
  message(FATAL_ERROR "BOOST_ROOT variable is defined but corresponds to non-existing directory")
endif()

string(REPLACE "^^" ";" MITK_USE_Boost_LIBRARIES "${MITK_USE_Boost_LIBRARIES}")

set(proj Boost)
set(proj_DEPENDENCIES )
set(Boost_DEPENDS ${proj})

if(NOT DEFINED BOOST_ROOT AND NOT MITK_USE_SYSTEM_Boost)

  set(_boost_version 1_60)
  set(_boost_install_include_dir include/boost)
  if(WIN32)
    set(_boost_install_include_dir include/boost-${_boost_version}/boost)
  endif()

  set(_boost_libs )
  set(_with_boost_libs )
  set(_install_lib_dir )

  # Set the boost root to the libraries install directory
  set(BOOST_ROOT "${ep_prefix}")

  if(MITK_USE_Boost_LIBRARIES)
    string(REPLACE ";" "," _boost_libs "${MITK_USE_Boost_LIBRARIES}")
    foreach(_boost_lib ${MITK_USE_Boost_LIBRARIES})
      list(APPEND _with_boost_libs --with-${_boost_lib})
    endforeach()
  endif()

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_boost_address_model "address-model=64")
  else()
    set(_boost_address_model "address-model=32")
  endif()

  if(WIN32)
    set(_shell_extension .bat)
    set(_boost_layout)
    if(MSVC)
      mitkFunctionGetMSVCVersion()
      set(_boost_with_toolset "vc${VISUAL_STUDIO_VERSION_MAJOR}")
      set(_boost_toolset "msvc-${VISUAL_STUDIO_VERSION_MAJOR}.0")
    endif()
    set(_install_lib_dir "--libdir=<INSTALL_DIR>/bin")
    set(WIN32_CMAKE_SCRIPT ${ep_prefix}/src/${proj}-cmake/MoveBoostLibsToLibDirForWindows.cmake)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeExternals/MoveBoostLibsToLibDirForWindows.cmake.in ${WIN32_CMAKE_SCRIPT} @ONLY)
    set(_windows_move_libs_cmd COMMAND ${CMAKE_COMMAND} -P ${WIN32_CMAKE_SCRIPT})
  else()
    set(_shell_extension .sh)
    set(_boost_layout "--layout=tagged")
  endif()

  if(UNIX AND NOT APPLE)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      set(_boost_with_toolset "gcc")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(_boost_with_toolset "clang")
    else()
      message(FATAL_ERROR "Compiler '${CMAKE_CXX_COMPILER_ID}' not supported. Use GNU or Clang instead.")
    endif()
      get_filename_component(_cxx_compiler_name "${CMAKE_CXX_COMPILER}" NAME)
      string(REGEX MATCH "^[0-9]+\\.[0-9]+" _compiler_version "${CMAKE_CXX_COMPILER_VERSION}")
      if(_cxx_compiler_name MATCHES "${_compiler_version}")
        set(_boost_toolset "${_boost_with_toolset}-${_compiler_version}")
      endif()
  endif()

  if(_boost_toolset)
    set(_boost_toolset "--toolset=${_boost_toolset}")
  endif()

  set (APPLE_SYSROOT_FLAG)
  if(APPLE)
    set(APPLE_CMAKE_SCRIPT ${ep_prefix}/src/${proj}-cmake/ChangeBoostLibsInstallNameForMac.cmake)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeExternals/ChangeBoostLibsInstallNameForMac.cmake.in ${APPLE_CMAKE_SCRIPT} @ONLY)
    set(_macos_change_install_name_cmd COMMAND ${CMAKE_COMMAND} -P ${APPLE_CMAKE_SCRIPT})

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
  set(_boost_cxxflags )
  if(CMAKE_CXX_FLAGS OR MITK_CXX11_FLAG)
    set(_boost_cxxflags "cxxflags=${MITK_CXX11_FLAG} ${CMAKE_CXX_FLAGS}")
  endif()
  set(_boost_linkflags )
  if(BUILD_SHARED_LIBS AND _install_rpath_linkflag)
    set(_boost_linkflags "linkflags=${_install_rpath_linkflag}")
  endif()

  set(_build_cmd "<SOURCE_DIR>/b2"
      ${APPLE_SYSROOT_FLAG}
      ${_boost_toolset}
      ${_boost_layout}
      "--prefix=<INSTALL_DIR>"
      ${_install_lib_dir}
      ${_with_boost_libs}
      # Use the option below to view the shell commands (for debugging)
      #-d+4
      variant=${_boost_variant}
      link=${_boost_link}
      ${_boost_cxxflags}
      ${_boost_linkflags}
      ${_boost_address_model}
      threading=multi
      runtime-link=shared
      # Some distributions site config breaks boost build
      # For example on Gentoo: http://stackoverflow.com/questions/23013433/how-to-install-modular-boost
      --ignore-site-config
      -q
  )

  if(MITK_USE_Boost_LIBRARIES)
    set(_boost_build_cmd BUILD_COMMAND ${_build_cmd})
    set(_install_cmd ${_build_cmd} install ${_macos_change_install_name_cmd} ${_windows_move_libs_cmd})
  else()
    set(_boost_build_cmd BUILD_COMMAND ${CMAKE_COMMAND} -E echo "no binary libraries")
    set(_install_cmd ${CMAKE_COMMAND} -E echo "copying Boost header..."
           COMMAND ${CMAKE_COMMAND} -E copy_directory "<SOURCE_DIR>/boost" "<INSTALL_DIR>/${_boost_install_include_dir}")
  endif()

  ExternalProject_Add(${proj}-download
      URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/boost_${_boost_version}_0.7z
      URL_MD5 7ce7f5a4e396484da8da6b60d4ed7661
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
    )

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    SOURCE_DIR "${ep_prefix}/src/${proj}-download"
    BINARY_DIR "${ep_prefix}/src/${proj}-download"
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND "<SOURCE_DIR>/bootstrap${_shell_extension}"
      --with-toolset=${_boost_with_toolset}
      --with-libraries=${_boost_libs}
      "--prefix=<INSTALL_DIR>"
    ${_boost_build_cmd}
    INSTALL_COMMAND ${_install_cmd}
    DEPENDS ${proj}-download ${proj_DEPENDENCIES}
    )

  ExternalProject_Get_Property(${proj} install_dir)

  if(WIN32)
    set(BOOST_LIBRARYDIR "${install_dir}/lib")
  endif()

  # Manual install commands (for a MITK super-build install)
  # until the Boost CMake system is used.

  # We just copy the include directory
  install(DIRECTORY "${install_dir}/${_boost_install_include_dir}"
          DESTINATION "include"
          COMPONENT dev
         )

  if(MITK_USE_Boost_LIBRARIES)
    # Copy the boost libraries
    file(GLOB _boost_libs
         "${install_dir}/lib/libboost*.so*"
         "${install_dir}/lib/libboost*.dylib")
    install(FILES ${_boost_libs}
            DESTINATION "lib"
            COMPONENT runtime)

    file(GLOB _boost_libs
         "${install_dir}/bin/libboost*.dll")
    install(FILES ${_boost_libs}
            DESTINATION "bin"
            COMPONENT runtime)

    file(GLOB _boost_libs
         "${install_dir}/lib/libboost*.lib"
         "${install_dir}/lib/libboost*.a")
    install(FILES ${_boost_libs}
            DESTINATION "lib"
            COMPONENT dev)
  endif()

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
