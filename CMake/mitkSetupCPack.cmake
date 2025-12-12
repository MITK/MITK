#
# First, set the generator variable
#
if(NOT CPACK_GENERATOR)
  if(WIN32)
    find_program(NSIS_MAKENSIS NAMES makensis
      PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
      DOC "Where is makensis.exe located"
      )

    set(CPACK_GENERATOR ZIP)

    if(NSIS_MAKENSIS)
      set(MITK_CREATE_NSIS_INSTALLER ON CACHE BOOL "Create NSIS installer in addition to ZIP archive")
      mark_as_advanced(MITK_CREATE_NSIS_INSTALLER)
      if(MITK_CREATE_NSIS_INSTALLER)
        list(APPEND CPACK_GENERATOR NSIS)
      endif()
    endif()
  else()
    if(APPLE)
      set(CPACK_GENERATOR DragNDrop)
    else()
      set(CPACK_GENERATOR TGZ)
    endif()
  endif()
endif()

# On windows set default install directory appropriately for 32 and 64 bit
# installers if not already set
if(WIN32 AND NOT CPACK_NSIS_INSTALL_ROOT)
  if(CMAKE_CL_64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  else()
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
  endif()
endif()

# By default, do not warn when built on machines using only VS Express
if(MITK_USE_OpenMP)
  set(CMAKE_INSTALL_OPENMP_LIBRARIES ON)
endif()

# include required mfc libraries
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "MITK")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Medical Imaging Interaction Toolkit")
set(CPACK_PACKAGE_VENDOR "German Cancer Research Center (DKFZ)")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${MITK_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_LICENSE "${MITK_SOURCE_DIR}/LICENSE")

string(REPLACE "/" "_" CPACK_PACKAGE_VERSION_MAJOR "${MITK_REVISION_DESC}")

# tell cpack to strip all debug symbols from all files
set(CPACK_STRIP_FILES ON)

# set version
if(NOT CPACK_PACKAGE_VERSION_MAJOR)
  set(CPACK_PACKAGE_VERSION_MAJOR ${MITK_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${MITK_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${MITK_VERSION_PATCH})
  set(CPACK_PACKAGE_VERSION "${MITK_VERSION_MAJOR}.${MITK_VERSION_MINOR}.${MITK_VERSION_PATCH}")
else()
  set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR})
endif()

# determine possible system specific extension
set(CPACK_PACKAGE_ARCH "unknown-architecture")

function(get_linux_name)
  set(linux_name "linux")
  set(os_release_file "/etc/os-release")

  if(EXISTS "${os_release_file}")
    file(STRINGS "${os_release_file}" os_release)

    set(name "")
    set(version_id "")

    foreach(line ${os_release})
      if(NOT name AND line MATCHES "^NAME=[\"]?([^\"]+)")
        set(name "${CMAKE_MATCH_1}")
      elseif(NOT version_id AND line MATCHES "^VERSION_ID=\"?([^\"]+)")
        set(version_id "${CMAKE_MATCH_1}")
      endif()

      if(name AND version_id)
        break()
      endif()
    endforeach()

    if(name)
      string(TOLOWER "${name}" name)
      string(REPLACE " " "_" name "${name}")
      set(linux_name "${name}")
    endif()

    if(version_id)
      set(linux_name "${linux_name}-${version_id}")
    endif()
  endif()

  set(${ARGV0} ${linux_name} PARENT_SCOPE)
endfunction()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(CPACK_PACKAGE_ARCH "windows-x86_64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  get_linux_name(linux_name)
  set(CPACK_PACKAGE_ARCH "${linux_name}-x86_64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CPACK_PACKAGE_ARCH "macos")
  if(CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CPACK_PACKAGE_ARCH "${CPACK_PACKAGE_ARCH}-${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()
  if(CMAKE_OSX_ARCHITECTURES)
    list(LENGTH CMAKE_OSX_ARCHITECTURES num_archs)
    if(num_archs EQUAL 1)
      set(CPACK_PACKAGE_ARCH "${CPACK_PACKAGE_ARCH}-${CMAKE_OSX_ARCHITECTURES}")
    else()
      set(CPACK_PACKAGE_ARCH "${CPACK_PACKAGE_ARCH}-universal")
    endif()
  endif()
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCH}")
