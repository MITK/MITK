#
# First, set the generator variable
#
if(NOT CPACK_GENERATOR)
  if(WIN32)
    find_program(NSIS_MAKENSIS NAMES makensis
      PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
      DOC "Where is makensis.exe located"
      )

    if(NOT NSIS_MAKENSIS)
      set(CPACK_GENERATOR ZIP)
    else()
      set(CPACK_GENERATOR "NSIS;ZIP")

    endif(NOT NSIS_MAKENSIS)
  else()
    if(APPLE)
      set(CPACK_GENERATOR DragNDrop)
    else()
      set(CPACK_GENERATOR TGZ)
    endif()
  endif()
endif(NOT CPACK_GENERATOR)

# Set Redistributable information for windows
if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  include(mitkFunctionGetMSVCVersion)
  mitkFunctionGetMSVCVersion()
  set(CPACK_VISUAL_STUDIO_VERSION_MAJOR "${VISUAL_STUDIO_VERSION_MAJOR}")
  set(CPACK_VISUAL_STUDIO_PRODUCT_NAME "${VISUAL_STUDIO_PRODUCT_NAME}")
  set(CPACK_LIBRARY_ARCHITECTURE "${CMAKE_LIBRARY_ARCHITECTURE}")

  # Visual Studio 2017 already comes with redistributable installers.
  # Try to find the right one.

  set(vswhere "$ENV{PROGRAMFILES\(X86\)}\\Microsoft Visual Studio\\Installer\\vswhere.exe")

  if(EXISTS ${vswhere})
    execute_process(COMMAND ${vswhere} -latest -property installationPath
      OUTPUT_VARIABLE installationPath
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    file(TO_CMAKE_PATH ${installationPath} installationPath)
    set(redistPath "${installationPath}/VC/Redist/MSVC")
    file(GLOB redistPath "${installationPath}/VC/Redist/MSVC/*")
    list(LENGTH redistPath length)
    if(length EQUAL 1)
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(redistPath "${redistPath}/vc_redist.x64.exe")
      else()
        set(redistPath "${redistPath}/vc_redist.x86.exe")
      endif()
      if(EXISTS ${redistPath})
        set(CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE ${redistPath} CACHE FILEPATH "Path to the appropriate Microsoft Visual Studio Redistributable")
      endif()
    endif()
  endif()

  if(NOT DEFINED CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE)
    set(CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE "" CACHE FILEPATH "Path to the appropriate Microsoft Visual Studio Redistributable")
  endif()
endif()

if(EXISTS ${CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE} )
  install(PROGRAMS ${CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE}
          DESTINATION thirdpartyinstallers)

  get_filename_component(CPACK_REDISTRIBUTABLE_FILE_NAME ${CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE} NAME )
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
if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
endif()

# include required mfc libraries
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "MITK")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MITK is a medical image processing tool")
set(CPACK_PACKAGE_VENDOR "German Cancer Research Center (DKFZ)")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${MITK_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${MITK_SOURCE_DIR}/LICENSE.txt")

set(CPACK_PACKAGE_VERSION_MAJOR ${MITK_REVISION_DESC})
string(REPLACE " [local changes]" "-local_changes" CPACK_PACKAGE_VERSION_MAJOR ${CPACK_PACKAGE_VERSION_MAJOR})

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
set(CPACK_PACKAGE_ARCH "unkown-architecture")

if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  if(CMAKE_CL_64)
    set(CPACK_PACKAGE_ARCH "windows-x86_64")
  elseif(WIN32)
    set(CPACK_PACKAGE_ARCH "windows-x86")
  endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
  if(${CMAKE_SYSTEM_PROCESSOR} MATCHES i686)
    set(CPACK_PACKAGE_ARCH "linux-x86")
  elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
    if(${CMAKE_CXX_FLAGS} MATCHES " -m32 ")
      set(CPACK_PACKAGE_ARCH "linux-x86")
    else()
      set(CPACK_PACKAGE_ARCH "linux-x86_64")
    endif()
  else()
    set(CPACK_PACKAGE_ARCH "linux")
  endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
  set(CPACK_PACKAGE_ARCH "macos-x86_64")
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCH}")



