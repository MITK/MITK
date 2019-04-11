#-----------------------------------------------------------------------------
# Boost
#-----------------------------------------------------------------------------

include(mitkFunctionGetMSVCVersion)

#[[ Sanity checks ]]
if(DEFINED BOOST_ROOT AND NOT EXISTS ${BOOST_ROOT})
  message(FATAL_ERROR "BOOST_ROOT variable is defined but corresponds to non-existing directory")
endif()

string(REPLACE "^^" ";" MITK_USE_Boost_LIBRARIES "${MITK_USE_Boost_LIBRARIES}")

set(proj Boost)
set(proj_DEPENDENCIES )
set(Boost_DEPENDS ${proj})

if(NOT DEFINED BOOST_ROOT AND NOT MITK_USE_SYSTEM_Boost)

  #[[ Reset variables. ]]
  set(patch_cmd "")
  set(configure_cmd "")
  set(install_cmd "")

  set(BOOST_ROOT ${ep_prefix})

  if(WIN32)
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
  endif()

  #[[ If you update Boost, make sure that the FindBoost module of the minimum
      required version of CMake supports the new version of Boost.

      In case you are using a higher version of CMake, download at least the
      source code of the minimum required version of CMake to look into the
      right version of the FindBoost module:

        <CMAKE_INSTALL_DIR>/share/cmake-<VERSION>/Modules/FindBoost.cmake

      Search for a list called _Boost_KNOWN_VERSIONS. If the new version is
      not included in this list, you have three options:

        * Update the minimum required version of CMake. This may require
          adaptions of other parts of our CMake scripts and has the most
          impact on other MITK developers. Yet this is the safest and
          cleanest option.

        * Set Boost_ADDITIONAL_VERSIONS (see the documentation of the
          FindBoost module). As Boost libraries and dependencies between
          them are hard-coded in the FindBoost module only for known versions,
          this may cause trouble for other MITK developers relying on new
          components of Boost or components with changed dependencies.

        * Copy a newer version of the FindBoost module into our CMake
          directory. Our CMake directory has a higher precedence than the
          default CMake module directory. Doublecheck if the minimum required
          version of CMake is able to process the newer version of the
          FindBoost module. Also, DO NOT FORGET to mention this option right
          above the call of cmake_minimum_required() in the top-level
          CMakeLists.txt file AND in this file right above the set(url)
          command below so if we update the minimum required version of CMake
          or use another option in the future, we do not forget to remove our
          copy of the FindBoost module again. ]]

  set(url "${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/boost_1_68_0.7z")
  set(md5 ae25f29cdb82cf07e8e26187ddf7d330)

  if(WIN32)
    #[[ See Task T25540 for details. Can be removed with Boost version greater
        1.68. In case the patch fails a "cd ." is executed to force the return
        return value to be 0 (success). We need this because we reuse the
        extracted source files and patching an already patched file returns
        an error code that we can ignore. ]]
    set(patch_cmd ${PATCH_COMMAND} --binary -N -p1 -i ${CMAKE_CURRENT_LIST_DIR}/Boost.patch || cd .)
  endif()



  if(MITK_USE_Boost_LIBRARIES)

     #[[ Boost has a two-step build process. In the first step, a bootstrap
         script is called to build b2, an executable that is used to actually
         build Boost in the second step.

         The bootstrap script expects a toolset (compiler) argument that is
         used to build the b2 executable. The scripts and their expected
         argument format differ between Windows and Unix. ]]

    if(WIN32)

      mitkFunctionGetMSVCVersion()

      if(VISUAL_STUDIO_VERSION_MINOR EQUAL 0)

        #[[ Use just the major version in the toolset name. ]]
        set(bootstrap_args vc${VISUAL_STUDIO_VERSION_MAJOR})

      elseif(VISUAL_STUDIO_VERSION_MAJOR EQUAL 14 AND VISUAL_STUDIO_VERSION_MINOR GREATER 0)

        #[[ There is no generic way of deducing the Boost toolset from the
            current minor version of Visual Studio 2017. All we can do for now
            is to assume that for all versions greater than 14.10 and less
            than 15.0 toolset vc141 is the right choice. ]]
        set(bootstrap_args vc${VISUAL_STUDIO_VERSION_MAJOR}1)

      else()

        #[[ Fallback to the generic case. Be prepared to add another elseif
            branch above for future versions of Visual Studio. ]]
        set(bootstrap_args vc${VISUAL_STUDIO_VERSION_MAJOR}${VISUAL_STUDIO_VERSION_MINOR})

      endif()

    else()

      #[[ We support GCC and Clang on Unix. On macOS, the toolset must be set
          to darwin. The actual compiler for all of these toolkits is set
          further below, after the bootstrap script but before b2. ]]

      if(APPLE)
        set(toolset darwin)
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
        set(toolset gcc)
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        set(toolset clang)
      endif()

      if(toolset)
        set(bootstrap_args --with-toolset=${toolset})
      endif()

      #[[ At least give it a shot if the toolset is something else and let
          the bootstrap script decide on the toolset by not passing any
          argument. ]]

    endif()

    #[[ The call of b2 is more complex. b2 arguments are grouped into options
        and properties. Options follow the standard format for arguments while
        properties are plain key-value pairs. ]]

    set(b2_options
      --build-dir=<BINARY_DIR>
      --stagedir=<INSTALL_DIR>
      --ignore-site-config #[[ Build independent of any site.config file ]]
      -q #[[ Stop at first error ]]
    )

    if(APPLE AND CMAKE_OSX_SYSROOT)

      #[[ Specify the macOS platform SDK to be used. ]]
      list(APPEND b2_options --sysroot=${CMAKE_OSX_SYSROOT})

    endif()

    foreach(lib ${MITK_USE_Boost_LIBRARIES})
      list(APPEND b2_options --with-${lib})
    endforeach()

    set(b2_properties
      threading=multi
      runtime-link=shared
      "cxxflags=${MITK_CXX14_FLAG} ${CMAKE_CXX_FLAGS}"
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      list(APPEND b2_properties address-model=64)
    else()
      list(APPEND b2_properties address-model=32)
    endif()

    if(BUILD_SHARED_LIBS)
      list(APPEND b2_properties link=shared)
    else()
      list(APPEND b2_properties link=static)
    endif()

    list(APPEND b2_properties "\
$<$<CONFIG:Debug>:variant=debug>\
$<$<CONFIG:Release>:variant=release>\
$<$<CONFIG:MinSizeRel>:variant=release>\
$<$<CONFIG:RelWithDebInfo>:variant=release>")

    if(WIN32)

      set(bootstrap_cmd if not exist b2.exe \( call bootstrap.bat ${bootstrap_args} \))
      set(b2_cmd b2 ${b2_options} ${b2_properties} stage)

    else()

      set(bootstrap_cmd #[[ test -e ./b2 || ]] ./bootstrap.sh ${bootstrap_args})
      set(b2_cmd ./b2 ${b2_options} ${b2_properties} stage)

      #[[ We already told Boost if we want to use GCC or Clang but so far we
          were not able to specify the exact same compiler we set in CMake
          when configuring the MITK superbuild for the first time.
          For example, this can be different from the system default
          when multiple versions of the same compiler are installed
          at the same time.

          The bootstrap script creates a configuration file for b2 that should
          be modified if necessary before b2 is called.
          We look for a line like

            using gcc ;

          and replace it with something more specific like

            using gcc : : /usr/bin/gcc-7.3 ;

          We use the stream editor sed for the replacement but as macOS is
          based on BSD Unix, we use the limited but portable BSD syntax
          instead of the more powerful GNU syntax. We also use | instead of
          the more commonly used / separator for sed because the replacement
          contains slashes. ]]

      if(toolset)
        set(configure_cmd sed -i.backup "\
s|\
using[[:space:]][[:space:]]*${toolset}[[:space:]]*$<SEMICOLON>|\
using ${toolset} : : ${CMAKE_CXX_COMPILER} $<SEMICOLON>|\
g"
          <SOURCE_DIR>/project-config.jam
        )
      endif()

    endif()

  endif()

  if(WIN32)
    set(dummy_cmd cd .)
  else()
    set(dummy_cmd true) #[[ "cd ." does not work reliably ]]
  endif()

  if(NOT patch_cmd)
    set(patch_cmd ${dummy_cmd}) #[[ Do nothing ]]
  endif()

  if(NOT configure_cmd)
    set(configure_cmd ${dummy_cmd}) #[[ Do nothing ]]
  endif()

  if(WIN32)
    set(install_cmd
      if not exist $<SHELL_PATH:${ep_prefix}/include/boost/config.hpp>
      \( ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/boost <INSTALL_DIR>/include/boost \)
    )
  else()
    set(install_cmd
      # test -e <INSTALL_DIR>/include/boost/config.hpp ||
      ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/boost <INSTALL_DIR>/include/boost
    )
  endif()

  ExternalProject_Add(${proj}
    URL ${url}
    URL_MD5 ${md5}
    PATCH_COMMAND ${patch_cmd}
    CONFIGURE_COMMAND ${configure_cmd}
    BUILD_COMMAND ""
    INSTALL_COMMAND ${install_cmd}
  )

  ExternalProject_Add_Step(${proj} bootstrap
    COMMAND ${bootstrap_cmd}
    DEPENDEES patch
    DEPENDERS configure
    WORKING_DIRECTORY <SOURCE_DIR>
  )

  ExternalProject_Add_Step(${proj} b2
    COMMAND ${b2_cmd}
    DEPENDEES bootstrap
    DEPENDERS build
    WORKING_DIRECTORY <SOURCE_DIR>
  )

  if(WIN32)

    #[[ Reuse already extracted files. ]]

    set(stamp_dir ${ep_prefix}/src/Boost-stamp)

    configure_file(
      ${CMAKE_CURRENT_LIST_DIR}/extract-Boost.replacement.cmake
      ${stamp_dir}/extract-Boost.replacement.cmake
      COPYONLY)

    ExternalProject_Add_Step(${proj} pre_download
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/Boost-pre_download.cmake
      DEPENDEES mkdir
      DEPENDERS download
      WORKING_DIRECTORY ${stamp_dir}
    )

  endif()

  if(MITK_USE_Boost_LIBRARIES)

    if(WIN32)

      #[[ Move DLLs from lib to bin directory. ]]

      ExternalProject_Add_Step(${proj} post_install
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/Boost-post_install-WIN32.cmake
        DEPENDEES install
        WORKING_DIRECTORY <INSTALL_DIR>/lib
      )

    elseif(APPLE)

      #[[ Boost does not follow the common practice of either using rpath or
          absolute paths for referencing dependencies. We have to use the
          install_name_tool to fix this. ]]

      ExternalProject_Add_Step(${proj} post_install
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/Boost-post_install-APPLE.cmake
        DEPENDEES install
        WORKING_DIRECTORY <INSTALL_DIR>/lib
      )

    endif()

  endif()

else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

endif()
