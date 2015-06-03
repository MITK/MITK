set(MITK_CONFIG_PACKAGES )

set(_apple_package_excludes)

if(APPLE)
  set(_apple_package_excludes
    SOFA
  )
endif()

set(_python_excludes )

if(NOT (CMAKE_SIZEOF_VOID_P EQUAL 8) )
  set(_python_excludes
    Python
    Numpy
   )
endif()

set(_package_excludes
  ${_apple_package_excludes}
  ${_python_excludes}
  OpenCL
  SYSTEM_Boost
  Boost_LIBRARIES
  SYSTEM_PYTHON
  SOFA_PLUGINS
  SOFA_PLUGINS_DIR
  SUPERBUILD
  BiophotonicsHardware_SpectroCam

  KWSTYLE
  MICROBIRD_TRACKER
  MICROBIRD_TRACKER_INCLUDE_DIR
  MICROBIRD_TRACKER_LIB
  MICRON_TRACKER
  OPTITRACK_TRACKER
  SPACENAVIGATOR
  TOF_KINECT
  TOF_KINECTV2
  TOF_MESASR4000
  TOF_PMDCAMBOARD
  TOF_PMDCAMCUBE
  TOF_PMDO3
  US_TELEMED_SDK
  videoInput
  WIIMOTE
)

get_cmake_property(_cache_vars CACHE_VARIABLES)
foreach(_cache_var ${_cache_vars})
  string(REGEX REPLACE "MITK_USE_(.+)" "\\1" _package "${_cache_var}")
  if(_package AND NOT _package STREQUAL _cache_var)
    list(FIND _package_excludes ${_package} _index)
    if(_index EQUAL -1)
      list(APPEND MITK_CONFIG_PACKAGES ${_package})
    endif()
  endif()
endforeach()

set(MITK_BUILD_ALL_APPS ON CACHE BOOL "Build all MITK applications" FORCE)
set(MITK_BUILD_ALL_PLUGINS ON CACHE BOOL "Build all MITK plugins" FORCE)
set(MITK_BUILD_EXAMPLES ON CACHE BOOL "Build the MITK examples" FORCE)
set(BLUEBERRY_BUILD_ALL_PLUGINS ON CACHE BOOL "Build all BlueBerry plugins" FORCE)
set(BUILD_DiffusionMiniApps ON CACHE BOOL "Build MITK Diffusion MiniApps" FORCE)
