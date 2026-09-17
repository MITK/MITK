#[[ Adds a python-build-standalone CPython as an external project:

      mitkFunctionAddStandalonePython(<proj> <version> <install_dir>
                                      [WITH_NUMPY] [CHECK_OPENSSL])

    <version> must be one of MITK_PYTHON3_VERSIONS; the matching patch version
    and archive hash come from Versions.cmake, which must be included
    beforehand. The archive is extracted and copied to <install_dir> verbatim,
    nothing is built.

    WITH_NUMPY provisions the packages MITK's Python code needs at runtime. An
    interpreter that only supplies headers and an import library for compiling
    extension modules does not need them.

    CHECK_OPENSSL verifies that the CPython build expects the OpenSSL version
    MITK found. Only relevant for an interpreter that is shipped and runs code
    opening TLS connections. ]]
function(mitkFunctionAddStandalonePython proj version install_dir)
  cmake_parse_arguments(PARSE_ARGV 3 ARG "WITH_NUMPY;CHECK_OPENSSL" "" "")

  if(ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "mitkFunctionAddStandalonePython: unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
  endif()

  set(script_dir "${MITK_SOURCE_DIR}/CMakeExternals/Python3")

  string(REPLACE "." "" version_key "${version}")

  if(WIN32)
    set(arch "x86_64")
    set(platform "pc-windows-msvc")
    set(platform_key "WIN")
  elseif(APPLE)
    set(platform "apple-darwin")
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
      set(arch "x86_64")
      set(platform_key "MAC_X86_64")
    else()
      set(arch "aarch64")
      set(platform_key "MAC_ARM64")
    endif()
  else()
    set(arch "x86_64_v2")
    set(platform "unknown-linux-gnu")
    set(platform_key "LINUX")
  endif()

  set(patch_version "${MITK_PYTHON3_${version_key}_PATCH}")
  set(release "${MITK_PYTHON3_RELEASE}")
  set(variant "install_only_stripped")

  set(base_url "https://github.com/astral-sh/python-build-standalone/releases/download/${release}")
  set(url "${base_url}/cpython-${patch_version}+${release}-${arch}-${platform}-${variant}.tar.gz")

  ExternalProject_Add(${proj}
    URL ${url}
    URL_HASH ${MITK_PYTHON3_${version_key}_HASH_${platform_key}}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E rm -rf "${install_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "<SOURCE_DIR>" "${install_dir}"
  )

  if(WIN32)
    set(python3_executable "<SOURCE_DIR>/python.exe")
  else()
    set(python3_executable "<SOURCE_DIR>/bin/python3")
  endif()

  if(ARG_CHECK_OPENSSL AND OPENSSL_VERSION)
    ExternalProject_Add_Step(${proj} check_openssl
      COMMAND ${python3_executable} "${script_dir}/Python3_CheckOpenSSL.py"
        --expected "${OPENSSL_VERSION}"
        --skip-if-built-in
      DEPENDEES patch
      DEPENDERS configure
      WORKING_DIRECTORY "<SOURCE_DIR>"
    )
  endif()

  if(ARG_WITH_NUMPY)
    if(CMAKE_OSX_ARCHITECTURES AND CMAKE_OSX_DEPLOYMENT_TARGET)
      ExternalProject_Add_Step(${proj} pip
        COMMAND ${python3_executable} "${script_dir}/Python3_macOS_numpy.py"
          --arch "${CMAKE_OSX_ARCHITECTURES}"
          --osx-target "${CMAKE_OSX_DEPLOYMENT_TARGET}"
        DEPENDEES patch
        DEPENDERS configure
        WORKING_DIRECTORY "<SOURCE_DIR>"
      )
    else()
      ExternalProject_Add_Step(${proj} pip
        COMMAND ${python3_executable} -s -m pip install --no-warn-script-location --upgrade pip
        COMMAND ${python3_executable} -s -m pip install --no-warn-script-location numpy~=2.3
        DEPENDEES patch
        DEPENDERS configure
        WORKING_DIRECTORY "<SOURCE_DIR>"
      )
    endif()
  endif()
endfunction()
