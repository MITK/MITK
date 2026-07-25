#------------------------------------------------------------
# Python Standalone Builds
#------------------------------------------------------------

#[[ Adds a python-build-standalone CPython as an external project:

      mitkFunctionAddStandalonePython(<proj> <version> <install_dir>
                                      [WITH_NUMPY] [CHECK_OPENSSL])

    <version> must be one of MITK_PYTHON3_VERSIONS; the matching patch version
    and archive hash come from Python3/Versions.cmake. The archive is extracted
    and copied to <install_dir> verbatim, nothing is built.

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
      COMMAND ${python3_executable} "${MITK_SOURCE_DIR}/CMakeExternals/Python3_CheckOpenSSL.py"
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
        COMMAND ${python3_executable} "${MITK_SOURCE_DIR}/CMakeExternals/Python3_macOS_numpy.py"
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

if(MITK_USE_Python3)
  set(proj Python3)
  set(proj_DEPENDENCIES )
  set(${proj}_DEPENDS ${proj})

  # Sanity checks
  if(DEFINED ${proj}_DIR AND NOT EXISTS "${${proj}_DIR}")
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    #[[ Patch versions and archive hashes per CPython series. Regenerate on a
        release bump with
        Python3/generate_versions.py <release> Python3/Versions.cmake ]]
    include("${CMAKE_CURRENT_LIST_DIR}/Python3/Versions.cmake")

    #[[ The CPython embedded in MITK. Deliberately lags the newest release: it
        has to run the AI segmentation tools, whose PyTorch dependency reaches a
        new CPython series later than the wheel-building infrastructure does.
        Independent of the CPython versions the mitk Python wheel targets. ]]
    set(MITK_Python3_VERSION "3.12" CACHE STRING "CPython version embedded in MITK")
    set_property(CACHE MITK_Python3_VERSION PROPERTY STRINGS ${MITK_PYTHON3_VERSIONS})
    mark_as_advanced(MITK_Python3_VERSION)

    if(NOT MITK_Python3_VERSION IN_LIST MITK_PYTHON3_VERSIONS)
      message(FATAL_ERROR "MITK_Python3_VERSION is \"${MITK_Python3_VERSION}\", but release "
        "${MITK_PYTHON3_RELEASE} provides these CPython versions: ${MITK_PYTHON3_VERSIONS}")
    endif()

    set(install_dir "${MITK_BINARY_DIR}/MITK-build/python")

    mitkFunctionAddStandalonePython(${proj} ${MITK_Python3_VERSION} "${install_dir}"
      WITH_NUMPY
      CHECK_OPENSSL
    )

    set(Python3_DIR ${install_dir})
    set(Python3_ROOT_DIR ${install_dir})

    #[[ CPython versions the mitk Python wheel is built for. pybind11 has no
        stable-ABI support, so one extension module per CPython series is
        unavoidable. Only that one target depends on the series, so the
        additional interpreters are staged for their headers and, on Windows,
        their import library. They are neither provisioned nor shipped, which is
        why they get neither of the function's options.

        Acted upon only by the PythonWheel configuration. Every other
        configuration builds no wheels, and downloading interpreters it would
        never compile against would be pure cost. ]]
    set(MITK_Python3_WHEEL_VERSIONS "${MITK_PYTHON3_VERSIONS}" CACHE STRING
      "CPython versions to build mitk Python wheels for")
    mark_as_advanced(MITK_Python3_WHEEL_VERSIONS)

    if(MITK_BUILD_CONFIGURATION STREQUAL "PythonWheel")
      foreach(wheel_version IN LISTS MITK_Python3_WHEEL_VERSIONS)
        if(NOT wheel_version IN_LIST MITK_PYTHON3_VERSIONS)
          message(FATAL_ERROR "MITK_Python3_WHEEL_VERSIONS contains \"${wheel_version}\", but release "
            "${MITK_PYTHON3_RELEASE} provides these CPython versions: ${MITK_PYTHON3_VERSIONS}")
        endif()
      endforeach()

      set(extra_wheel_versions ${MITK_Python3_WHEEL_VERSIONS})
      list(REMOVE_ITEM extra_wheel_versions ${MITK_Python3_VERSION})

      if(extra_wheel_versions AND NOT MITK_USE_pybind11)
        message(FATAL_ERROR "MITK_Python3_WHEEL_VERSIONS asks for wheels beyond the embedded CPython "
          "${MITK_Python3_VERSION}, but building Python wheels requires MITK_USE_pybind11")
      endif()

      foreach(wheel_version IN LISTS extra_wheel_versions)
        set(wheel_proj "Python3-${wheel_version}")

        mitkFunctionAddStandalonePython(${wheel_proj} ${wheel_version}
          "${MITK_BINARY_DIR}/MITK-build/python-${wheel_version}")

        list(APPEND ${proj}_DEPENDS ${wheel_proj})
      endforeach()
    endif()
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
