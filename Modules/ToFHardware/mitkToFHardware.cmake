#option MITK_USE_TOF_HARDWARE

if(WIN32)
  if(CMAKE_CL_64)
    set(_PLATFORM_STRING "W64")
  else(CMAKE_CL_64)
    set(_PLATFORM_STRING "W32")
  endif(CMAKE_CL_64)
else(WIN32)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(_PLATFORM_STRING "L64")
  else(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(_PLATFORM_STRING "L32")
  endif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
endif(WIN32)

set(MITK_TOF_AVAILABLE_CAMERAS MITK Player)

option(MITK_ENABLE_TOF_HARDWARE "Support for range cameras" OFF)
if(MITK_ENABLE_TOF_HARDWARE)
	
#Begin PMD Camcube hardware
option(MITK_USE_TOF_PMDCAMCUBE "Enable support for PMD camcube" OFF)

# only if PMD Camcube is enabled
if(MITK_USE_TOF_PMDCAMCUBE)
  find_library(MITK_PMD_LIB pmdaccess2 DOC "PMD access library.")
  get_filename_component(MITK_PMD_SDK_DIR ${MITK_PMD_LIB} PATH)
  set(MITK_PMD_SDK_DIR ${MITK_PMD_SDK_DIR}/..)
  find_path(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")
  set(MITK_TOF_PMD_CAMCUBE_SOURCE ${MITK_PMD_SDK_DIR}/plugins/camcube3.${_PLATFORM_STRING}.pap)
  set(MITK_TOF_PMD_CAMCUBE_PROC ${MITK_PMD_SDK_DIR}/plugins/camcubeproc.${_PLATFORM_STRING}.ppp)
  set(MITK_TOF_PMD_FILE_SOURCE ${MITK_PMD_SDK_DIR}/plugins/pmdfile.${_PLATFORM_STRING}.pcp)
  set(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD CamCube 2.0/3.0,PMD CamCubeRaw 2.0/3.0,PMD Player,PMD Raw Data Player)
#  file(GLOB MITK_TOF_PMD_CAMCUBE_SOURCE camcube3.${_PLATFORM_STRING}.pap)
#  file(GLOB MITK_TOF_PMD_CAMCUBE_PROC camcubeproc.${_PLATFORM_STRING}.ppp)
#  file(GLOB MITK_TOF_PMD_FILE_SOURCE pmdfile.${_PLATFORM_STRING}.pcp)
  if(WIN32)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/bin/pmdaccess2.dll  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcube3.W32.pap  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcubeproc.W32.ppp  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcube3.W64.pap  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcubeproc.W64.ppp  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/pmdfile.W32.pcp  CONFIGURATIONS Release)
  endif(WIN32)
endif(MITK_USE_TOF_PMDCAMCUBE)
#End PMD Camcube Hardware

#Begin PMD CamBoard hardware
option(MITK_USE_TOF_PMDCAMBOARD "Enable support for PMD camboard" OFF)

# only if PMD CamBoard is enabled
if(MITK_USE_TOF_PMDCAMBOARD)
  find_library(MITK_PMD_LIB pmdaccess2 DOC "PMD access library.")
  get_filename_component(MITK_PMD_SDK_DIR ${MITK_PMD_LIB} PATH)
  set(MITK_PMD_SDK_DIR ${MITK_PMD_SDK_DIR}/..)
  find_path(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")
  set(MITK_TOF_PMD_CAMBOARD_SOURCE ${MITK_PMD_SDK_DIR}/plugins/camboard.${_PLATFORM_STRING}.pap)
  set(MITK_TOF_PMD_CAMBOARD_PROC ${MITK_PMD_SDK_DIR}/plugins/camboardproc.${_PLATFORM_STRING}.ppp)
  set(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD CamBoard,PMD CamBoardRaw)
#  file(GLOB MITK_TOF_PMD_CAMBOARD_SOURCE camboard.${_PLATFORM_STRING}.pap)
#  file(GLOB MITK_TOF_PMD_CAMBOARD_PROC camboardproc.${_PLATFORM_STRING}.ppp)
#  file(GLOB MITK_TOF_PMD_FILE_SOURCE ${MITK_PMD_SDK_DIR}/plugins/pmdfile.${_PLATFORM_STRING}.pcp)
  if(WIN32)
   install(FILES ${MITK_PMD_SDK_DIR}/bin/pmdaccess2.dll DESTINATION bin CONFIGURATIONS Release)
   install(FILES ${MITK_PMD_SDK_DIR}/plugins/camboard.W32.pap DESTINATION bin CONFIGURATIONS Release)
   install(FILES ${MITK_PMD_SDK_DIR}/plugins/camboardproc.W32.ppp DESTINATION bin CONFIGURATIONS Release)
#   install(FILES ${MITK_PMD_SDK_DIR}/plugins/camboard.W64.pap DESTINATION bin CONFIGURATIONS Release)
#   install(FILES ${MITK_PMD_SDK_DIR}/plugins/camboardproc.W64.ppp DESTINATION bin CONFIGURATIONS Release)
#   install(FILES ${MITK_PMD_SDK_DIR}/plugins/pmdfile.W32.pcp DESTINATION bin CONFIGURATIONS Release)
  endif(WIN32)
endif(MITK_USE_TOF_PMDCAMBOARD)
#End PMD CamBoard Hardware

#Begin MESA SR4000 hardware
option(MITK_USE_TOF_MESASR4000 "Enable support for MESA SR4000" OFF)

# only if MESA SR4000 is enabled
if(MITK_USE_TOF_MESASR4000)
  find_library(MITK_MESA_LIB libMesaSR DOC "MESA access library.")
  get_filename_component(MITK_MESA_SDK_DIR ${MITK_MESA_LIB} PATH)
  set(MITK_MESA_SDK_DIR ${MITK_MESA_SDK_DIR}/..)
  find_path(MITK_MESA_INCLUDE_DIR libMesaSR.h ${MITK_MESA_SDK_DIR}/include DOC  "Include directory of MESA-SDK.")
  set(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},MESA Swissranger 4000)
  if(WIN32)
   install(FILES ${MITK_MESA_SDK_DIR}/bin/libMesaSR.dll DESTINATION bin CONFIGURATIONS Release)
  endif(WIN32)
endif(MITK_USE_TOF_MESASR4000)
#End MESA SR4000 Hardware

if(WIN32)

#Begin PMD O3 hardware
option(MITK_USE_TOF_PMDO3 "Enable support for PMD O3 camera" OFF)
# only if PMD O3 is enabled
if(MITK_USE_TOF_PMDO3)
  find_library(MITK_PMD_LIB pmdaccess2 DOC "PMD access library.")
  get_filename_component(MITK_PMD_SDK_DIR ${MITK_PMD_LIB} PATH)
  set(MITK_PMD_SDK_DIR ${MITK_PMD_SDK_DIR}/..)
  find_path(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")
  set(MITK_TOF_PMD_O3D_SOURCE ${MITK_PMD_SDK_DIR}/plugins/o3d.${_PLATFORM_STRING}.pcp)
  set(MITK_TOF_PMD_O3D_PROC ${MITK_PMD_SDK_DIR}/plugins/o3d.${_PLATFORM_STRING}.pcp)
  set(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD O3D)
  
#  file(GLOB MITK_TOF_PMD_O3D_SOURCE o3d.${_PLATFORM_STRING}.pcp)
#  file(GLOB MITK_TOF_PMD_O3D_PROC o3d.${_PLATFORM_STRING}.pcp)
  MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/bin/pmdaccess2.dll  CONFIGURATIONS Release)
  MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/o3d.W32.pcp  CONFIGURATIONS Release)
endif(MITK_USE_TOF_PMDO3)
#End PMD O3 Hardware
endif(WIN32)

endif(MITK_ENABLE_TOF_HARDWARE)

configure_file(mitkToFConfig.h.in ${PROJECT_BINARY_DIR}/mitkToFConfig.h @ONLY)

