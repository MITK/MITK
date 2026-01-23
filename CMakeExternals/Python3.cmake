#------------------------------------------------------------
# Python Standalone Builds
#------------------------------------------------------------

if(MITK_USE_Python3)
  set(proj Python3)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  # Sanity checks
  if(DEFINED ${proj}_DIR AND NOT EXISTS "${${proj}_DIR}")
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    set(version 3.12.12)       # IMPORTANT: On any version update, adapt MITK_REQUIRED_OPENSSL_VERSION
    set(release_date 20260114) #            in the top-level CMakeLists.txt.

    set(base_url "https://github.com/astral-sh/python-build-standalone/releases/download/${release_date}")
    set(variant "install_only_stripped")

    if(WIN32)
      set(platform "pc-windows-msvc")
      set(arch "x86_64")
      set(url_hash "SHA256=65544affdc45a3755db3a08fd0b36c5b590bb49337b99a19b8840c33189fe75e")
    elseif(APPLE)
      set(platform "apple-darwin")
      if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(arch "x86_64")
        set(url_hash "SHA256=bcd4bc53a5f7d6baa2976a62fd95c6907f86c4ad7ae220c2ee1f49deba8c6d1c")
      else()
        set(arch "aarch64")
        set(url_hash "SHA256=ed1f300bd3b45aa481d887b2dd8e12f989b583f67755c219a6092756a09b609f")
      endif()
    else()
      set(platform "unknown-linux-gnu")
      set(arch "x86_64_v2")
      set(url_hash "SHA256=3426df806b8072a80e6adf88de005d7f010ea90ddbea1d1d725118c5ab04ac5a")
    endif()

    set(url "${base_url}/cpython-${version}+${release_date}-${arch}-${platform}-${variant}.tar.gz")
    set(install_dir "${MITK_BINARY_DIR}/MITK-build/python")

    ExternalProject_Add(${proj}
      URL ${url}
      URL_HASH ${url_hash}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ${CMAKE_COMMAND} -E rm -rf "${install_dir}"
              COMMAND ${CMAKE_COMMAND} -E copy_directory "<SOURCE_DIR>" "${install_dir}"
      DEPENDS "${proj_DEPENDENCIES}"
    )

    if(WIN32)
      set(python3_executable "python.exe")
    else()
      set(python3_executable "bin/python3")
    endif()

    if(OPENSSL_VERSION)
      ExternalProject_Add_Step(${proj} check_openssl
        COMMAND ${python3_executable} "${MITK_SOURCE_DIR}/CMakeExternals/Python3_CheckOpenSSL.py"
          --expected "${OPENSSL_VERSION}"
          --skip-if-built-in
        DEPENDEES patch
        DEPENDERS configure
        WORKING_DIRECTORY "<SOURCE_DIR>"
      )
    endif()

    if(CMAKE_OSX_ARCHITECTURES AND CMAKE_OSX_DEPLOYMENT_TARGET)
      ExternalProject_Add_Step(${proj} pip
        COMMAND ${python3_executable} "${MITK_SOURCE_DIR}/CMakeExternals/Python3_macOS_numpy.py"
          --arch "${CMAKE_OSX_ARCHITECTURES}"
          --osx-target "${CMAKE_OSX_DEPLOYMENT_TARGET}"
        DEPENDEES patch
        DEPENDERS configure
        WORKING_DIRECTORY "<SOURCE_DIR>"
      )
    else()
      ExternalProject_Add_Step(${proj} pip
        COMMAND ${python3_executable} -m pip install --no-warn-script-location --upgrade pip
        COMMAND ${python3_executable} -m pip install --no-warn-script-location numpy~=2.3
        DEPENDEES patch
        DEPENDERS configure
        WORKING_DIRECTORY "<SOURCE_DIR>"
      )
    endif()

    set(Python3_DIR ${install_dir})
    set(Python3_ROOT_DIR ${install_dir})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
