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

  set(_boost_version 1_68)
  set(_boost_install_include_dir include/boost)
  if(WIN32)
    set(_boost_install_include_dir include/boost-${_boost_version}/boost)
  endif()

  set(_boost_libs )
  set(_with_boost_libs )
  set(_bootstrap_args )
  set(_install_lib_dir )
  set(_env )

  # Set the boost root to the libraries install directory
  set(BOOST_ROOT "${ep_prefix}")

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_boost_address_model "64")
  else()
    set(_boost_address_model "32")
  endif()

  if(MITK_USE_Boost_LIBRARIES)
    string(REPLACE ";" "," _boost_libs "${MITK_USE_Boost_LIBRARIES}")
    string(REGEX REPLACE "python[0-9]*" "python"  _bootstrap_libs "${_boost_libs}")
    foreach(_boost_lib ${MITK_USE_Boost_LIBRARIES})
      if (_boost_lib MATCHES "python[0-9]*")
        find_package (Python2 COMPONENTS Interpreter Development)
        find_package (Python3 COMPONENTS Interpreter Development)
        string(REGEX REPLACE "([^.]*\\.[^.]*)\\..*" "\\1" _p2v "${Python2_VERSION}")
        string(REGEX REPLACE "([^.]*\\.[^.]*)\\..*" "\\1" _p3v "${Python3_VERSION}")
        if(Python2_FOUND)
          string(REGEX REPLACE "([\\ ])" "\\\\\\1" _pe "${Python2_EXECUTABLE}")
          string(REGEX REPLACE "([\\ ])" "\\\\\\1" _pi "${Python2_INCLUDE_DIRS}")
          string(REGEX REPLACE "([\\ ])" "\\\\\\1" _pl "${Python2_LIBRARY_DIRS}")
#         list(APPEND _env "USINGP2=using python : ${_p2v} : ${_pe} : ${_pi} : ${_pl} : <address-model>${_boost_address_model} <address-model>")
          list(APPEND _env "USINGP2=using python : ${_p2v} : ${_pe} : ${_pi} : ${_pl}")
        else()
          list(APPEND _env "USINGP2=# no python2")
        endif()
        if(Python3_FOUND)
          string(REGEX REPLACE "([\\ ])" "\\\\\\1" _pe "${Python3_EXECUTABLE}")
          string(REGEX REPLACE "([\\ ])" "\\\\\\1" _pi "${Python3_INCLUDE_DIRS}")
          string(REGEX REPLACE "([\\ ])" "\\\\\\1" _pl "${Python3_LIBRARY_DIRS}")
#         list(APPEND _env "USINGP3=using python : ${_p3v} : ${_pe} : ${_pi} : ${_pl} : <address-model>${_boost_address_model} <address-model>")
          list(APPEND _env "USINGP3=using python : ${_p3v} : ${_pe} : ${_pi} : ${_pl}")
        else()
          list(APPEND _env "USINGP3=# no python3")
        endif()
        list(APPEND _with_boost_libs --with-python)
        if(Python3_FOUND AND Python2_FOUND)
          if(WIN32)
            get_filename_component(_python_basedir ${Python2_INCLUDE_DIRS} DIRECTORY)
            list(APPEND _bootstrap_args "--with-python=${_python_basedir}")
          else()
            list(APPEND _bootstrap_args "--with-python=${Python2_EXECUTABLE}")
          endif()
          list(APPEND _with_boost_libs "python=${_p2v},${_p3v}")
        elseif(Python2_FOUND)
          if(WIN32)
            get_filename_component(_python_basedir ${Python2_INCLUDE_DIRS} DIRECTORY)
            list(APPEND _bootstrap_args "--with-python=${_python_basedir}")
          else()
            list(APPEND _bootstrap_args "--with-python=${Python2_EXECUTABLE}")
          endif()
          list(APPEND _with_boost_libs "python=${_p2v}")
        elseif(Python3_FOUND)
          if(WIN32)
            get_filename_component(_python_basedir ${Python3_INCLUDE_DIRS} DIRECTORY)
            list(APPEND _bootstrap_args "--with-python=${_python_basedir}")
          else()
            list(APPEND _bootstrap_args "--with-python=${Python3_EXECUTABLE}")
          endif()
          list(APPEND _with_boost_libs "python=${_p3v}")
        endif()
      else()
        list(APPEND _with_boost_libs --with-${_boost_lib})
      endif()
    endforeach()
  endif()

  if(WIN32)
    set(_shell_extension .bat)
    set(_patch2 COMMAND ${PATCH_COMMAND} -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Boost-win.patch)
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
    set(_patch2 )
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

  set(_boost_variant "$<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>$<$<CONFIG:RelWithDebInfo>:release>")
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
      address-model=${_boost_address_model}
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

  ExternalProject_Add(${proj}
    LIST_SEPARATOR ${sep}
    URL ${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/boost_${_boost_version}_0.7z
    URL_MD5 ae25f29cdb82cf07e8e26187ddf7d330
    PATCH_COMMAND ${PATCH_COMMAND} -l -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Boost.patch
      ${_patch2}
    BINARY_DIR "${ep_prefix}/src/${proj}"
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${_env} "<SOURCE_DIR>/bootstrap${_shell_extension}"
      --with-toolset=${_boost_with_toolset}
      --with-libraries=${_bootstrap_libs}
      ${_bootstrap_args}
      "--prefix=<INSTALL_DIR>"
    ${_boost_build_cmd}
    INSTALL_COMMAND ${_install_cmd}
    DEPENDS ${proj_DEPENDENCIES}
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
         "${install_dir}/bin/boost*.dll")
    install(FILES ${_boost_libs}
            DESTINATION "../ep/bin")
            
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
