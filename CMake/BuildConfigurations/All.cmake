set(MITK_CONFIG_PACKAGES )

set(_apple_package_excludes)

set(_package_excludes
  ${_apple_package_excludes}
  OpenCL
  SYSTEM_Boost
  Boost_LIBRARIES
  SYSTEM_PYTHON
  SUPERBUILD
  BiophotonicsHardware_SpectroCam
  POLHEMUS_TRACKER

  US_DiPhAS_SDK
  GALIL_HARDWARE
  OPHIR_PYRO_HARDWARE

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

set(BUILD_DiffusionCoreCmdApps ON CACHE BOOL "Build commandline tools for diffusion" FORCE)
set(BUILD_DiffusionFiberProcessingCmdApps ON CACHE BOOL "Build commandline tools for diffusion fiber processing" FORCE)
set(BUILD_DiffusionFiberfoxCmdApps ON CACHE BOOL "Build commandline tools for diffusion data simulation (Fiberfox)" FORCE)
set(BUILD_DiffusionMiscCmdApps ON CACHE BOOL "Build miscellaneous commandline tools for diffusion" FORCE)
set(BUILD_DiffusionQuantificationCmdApps ON CACHE BOOL "Build commandline tools for diffusion quantification (IVIM, ADC, ...)" FORCE)
set(BUILD_DiffusionTractographyCmdApps ON CACHE BOOL "Build commandline tools for diffusion fiber tractography" FORCE)
set(BUILD_DiffusionTractographyEvaluationCmdApps ON CACHE BOOL "Build commandline tools for diffusion fiber tractography evaluation" FORCE)
set(BUILD_DiffusionConnectomicsCmdApps ON CACHE BOOL "Build commandline tools for diffusion connectomics" FORCE)
