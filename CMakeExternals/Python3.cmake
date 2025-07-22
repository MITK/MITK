#------------------------------------------------------------
# Python Standalone Builds
#------------------------------------------------------------

set(proj Python3)
set(proj_DEPENDENCIES )
set(${proj}_DEPENDS ${proj})

if(MITK_USE_${proj})
  # Sanity checks
  if(DEFINED ${proj}_DIR AND NOT EXISTS "${${proj}_DIR}")
    message(FATAL_ERROR "${proj}_DIR variable is defined but corresponds to non-existing directory")
  endif()

  if(NOT DEFINED ${proj}_DIR)
    set(version 3.12.11)
    set(release_date 20250712)

    set(pip_install
      numpy
    )

    set(base_url "https://github.com/astral-sh/python-build-standalone/releases/download/${release_date}")
    set(variant "install_only_stripped")

    if(WIN32)
      set(platform "pc-windows-msvc")
      set(arch "x86_64")
      set(url_hash "SHA256=49911a479230f9a0ad33fc6742229128249f695502360dab3f5fd9096585e9a5")
    elseif(APPLE)
      set(platform "apple-darwin")
      if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(arch "x86_64")
        set(url_hash "SHA256=1154b0be69bdd8c144272cee596181f096577d535bff1548f8df49e0d7d9c721")
      else()
        set(arch "aarch64")
        set(url_hash "SHA256=0a5748a455ebd0ef0419bffa0b239c1596ea021937fa4c9eb3b8893cf7b46d48")
      endif()
    else()
      set(platform "unknown-linux-gnu")
      set(arch "x86_64_v2")
      set(url_hash "SHA256=ddf0c26a2df22156672e7476fda10845056d13d4b5223de6ba054d25bfcd9d3c")
    endif()

    set(url "${base_url}/cpython-${version}+${release_date}-${arch}-${platform}-${variant}.tar.gz")

    if(WIN32)
      set(python3_executable "<SOURCE_DIR>/python.exe")
    else()
      set(python3_executable "<SOURCE_DIR>/bin/python3")
    endif()

    list(JOIN pip_install " " packages)

    ExternalProject_Add(${proj}
      URL ${url}
      URL_HASH ${url_hash}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ${python3_executable} -m pip install --upgrade pip --no-warn-script-location
              COMMAND ${python3_executable} -m pip install ${packages} --no-warn-script-location
      DEPENDS "${proj_DEPENDENCIES}"
    )

    ExternalProject_Get_Property(${proj} SOURCE_DIR)
    set(Python3_DIR ${SOURCE_DIR})
    set(Python3_ROOT_DIR ${SOURCE_DIR})
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()
