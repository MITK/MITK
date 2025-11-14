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
    set(version 3.12.12)
    set(release_date 20251031)

    set(base_url "https://github.com/astral-sh/python-build-standalone/releases/download/${release_date}")
    set(variant "install_only_stripped")

    if(WIN32)
      set(platform "pc-windows-msvc")
      set(arch "x86_64")
      set(url_hash "SHA256=6871bee00f8055ef7e210523cb34a727cff7df03261d6e0b8d405592edab8f1e")
    elseif(APPLE)
      set(platform "apple-darwin")
      if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(arch "x86_64")
        set(url_hash "SHA256=10538a63ce7aa4d0891a3181150159fb5d3b750470a638ef8255a25dc50b9961")
      else()
        set(arch "aarch64")
        set(url_hash "SHA256=2539a9105e47237aa8caabeed20587778f4acf9c6c4b3fe77f9bc2fd320720d2")
      endif()
    else()
      set(platform "unknown-linux-gnu")
      set(arch "x86_64_v2")
      set(url_hash "SHA256=009534a3a6a0b78054fd9c93943d894a89a8ee3318501c92a0b9684e903bdda9")
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
