#option MITK_USE_TOF_HARDWARE
IF(MITK_ENABLE_TOF_HARDWARE)

#--------------------------------------Checking Os--------------------------------------------



###-------------------------------Begin CamCube hardware-------------------------------------------
OPTION(MITK_USE_TOF_PMDCAMCUBE "Enable support for Cam Cube camera" OFF)
# only if PMD Camcube is enabled
IF(MITK_USE_TOF_PMDCAMCUBE)
  FIND_LIBRARY(MITK_PMD_LIB pmdaccess2 DOC "PMD access library.")
  GET_FILENAME_COMPONENT(MITK_PMD_SDK_DIR ${MITK_PMD_LIB} PATH)
  SET(MITK_PMD_SDK_DIR ${MITK_PMD_SDK_DIR}/..)
  FIND_PATH(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")
  SET(MITK_TOF_PMD_CAMCUBE_SOURCE ${MITK_PMD_SDK_DIR}/plugins/camcube3.${_PLATFORM_STRING}.pap)
  SET(MITK_TOF_PMD_CAMCUBE_PROC ${MITK_PMD_SDK_DIR}/plugins/camcubeproc.${_PLATFORM_STRING}.ppp)
  SET(MITK_TOF_PMD_FILE_SOURCE ${MITK_PMD_SDK_DIR}/plugins/pmdfile.${_PLATFORM_STRING}.pcp)
  SET(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD CamCube 2.0/3.0,PMD CamCubeRaw 2.0/3.0,PMD Player,PMD Raw Data Player)

  set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_PMD_SDK_DIR}/include)
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_PMD_LIB})

  IF(WIN32)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/bin/pmdaccess2.dll  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcube3.W32.pap  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcubeproc.W32.ppp  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcube3.W64.pap  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camcubeproc.W64.ppp  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/pmdfile.W32.pcp  CONFIGURATIONS Release)
  ENDIf(WIN32)

#Keinerlei Abfrage ueber Raw Data CamCube

MITK_CREATE_MODULE(mitkPMDCamCubeModule
  SUBPROJECTS MITK-ToF
  INCLUDE_DIRS ${MITK_BIN_DIR}
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  DEPENDS mitkToFHardware
  EXPORT_DEFINE MITK_PMDCAMCUBEMODULE_EXPORT
  ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
)
  Message("PMD CamCube Module erfolgreich erstellt")

#add_subdirectory(Testing)
ENDIF(MITK_USE_TOF_PMDCAMCUBE)

###--------------------------------------------------------------------------------------------------------



##-------------------------------Begin CamBoard hardware-------------------------------------------
 OPTION(MITK_USE_TOF_PMDCAMBOARD "Enable support for PMD camboard" OFF)



## only if PMD CamBoard is enabled
 IF(MITK_USE_TOF_PMDCAMBOARD)
   FIND_LIBRARY(MITK_PMD_LIB pmdaccess2 DOC "PMD access library.")
   GET_FILENAME_COMPONENT(MITK_PMD_SDK_DIR ${MITK_PMD_LIB} PATH)
   SET(MITK_PMD_SDK_DIR ${MITK_PMD_SDK_DIR}/..)
   FIND_PATH(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")
   SET(MITK_TOF_PMD_CAMBOARD_SOURCE ${MITK_PMD_SDK_DIR}/plugins/camboard.${_PLATFORM_STRING}.pap)
   SET(MITK_TOF_PMD_CAMBOARD_PROC ${MITK_PMD_SDK_DIR}/plugins/camboardproc.${_PLATFORM_STRING}.ppp)
   SET(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD CamBoard,PMD CamBoardRaw)
#  FILE(GLOB MITK_TOF_PMD_CAMBOARD_SOURCE camboard.${_PLATFORM_STRING}.pap)
#  FILE(GLOB MITK_TOF_PMD_CAMBOARD_PROC camboardproc.${_PLATFORM_STRING}.ppp)
#  FILE(GLOB MITK_TOF_PMD_FILE_SOURCE ${MITK_PMD_SDK_DIR}/plugins/pmdfile.${_PLATFORM_STRING}.pcp)
  IF(WIN32)
    INSTALL(FILES ${MITK_PMD_SDK_DIR}/bin/pmdaccess2.dll DESTINATION bin CONFIGURATIONS Release)
    INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboard.W32.pap DESTINATION bin CONFIGURATIONS Release)
    INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboardproc.W32.ppp DESTINATION bin CONFIGURATIONS Release)
#   INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboard.W64.pap DESTINATION bin CONFIGURATIONS Release)
#   INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/camboardproc.W64.ppp DESTINATION bin CONFIGURATIONS Release)
#   INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/pmdfile.W32.pcp DESTINATION bin CONFIGURATIONS Release)
   ENDIf(WIN32)

 ##---------------------------------????????????????????????????????????????????????????????????????????????
   MITK_CREATE_MODULE(mitkPMDCamBoardModule
   SUBPROJECTS MITK-ToF
   INCLUDE_DIRS ${MITK_BIN_DIR}
   INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
   DEPENDS mitkToFHardware
   EXPORT_DEFINE MITK_PMDCAMBOARDMODULE_EXPORT
   ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
 )
   Message("PMD CamBoard Module erfolgreich erstellt")

 #add_subdirectory(Testing)
 ENDIF(MITK_USE_TOF_PMDCAMBOARD)
##--------------------------------------------------------------------------------------------------------


##----------------------------------------Begin PMD O3 hardware--------------------------------------------
 IF (WIN32)

 OPTION(MITK_USE_TOF_PMDO3 "Enable support for PMD O3 camera" OFF)

# only if PMD O3 is enabled
 IF(MITK_USE_TOF_PMDO3)
   FIND_LIBRARY(MITK_PMD_LIB pmdaccess2 DOC "PMD access library.")
   GET_FILENAME_COMPONENT(MITK_PMD_SDK_DIR ${MITK_PMD_LIB} PATH)
   SET(MITK_PMD_SDK_DIR ${MITK_PMD_SDK_DIR}/..)
   FIND_PATH(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")
   SET(MITK_TOF_PMD_O3D_SOURCE ${MITK_PMD_SDK_DIR}/plugins/o3d.${_PLATFORM_STRING}.pcp)
   SET(MITK_TOF_PMD_O3D_PROC ${MITK_PMD_SDK_DIR}/plugins/o3d.${_PLATFORM_STRING}.pcp)
   SET(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD O3D)

   FILE(GLOB MITK_TOF_PMD_O3D_SOURCE o3d.${_PLATFORM_STRING}.pcp)
   FILE(GLOB MITK_TOF_PMD_O3D_PROC o3d.${_PLATFORM_STRING}.pcp)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/bin/pmdaccess2.dll  CONFIGURATIONS Release)
   MITK_INSTALL(FILES ${MITK_PMD_SDK_DIR}/plugins/o3d.W32.pcp  CONFIGURATIONS Release)


   MITK_CREATE_MODULE(mitkPMDO3Module
   SUBPROJECTS MITK-ToF
   INCLUDE_DIRS ${MITK_BIN_DIR}
   INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
   DEPENDS mitkToFHardware
   EXPORT_DEFINE MITK_PMD03MODULE_EXPORT
   ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
 )
   Message("PMD O3 Module erfolgreich erstellt")

 #add_subdirectory(Testing)

 ENDIF(MITK_USE_TOF_PMDO3)
 ENDIF(WIN32)
##--------------------------------------------------------------------------------------------------------


#-----------------------------------Creating a "Testing"-subdirectory------------------------------
IF(MITK_USE_TOF_PMDO3 OR MITK_USE_TOF_PMDCAMCUBE OR MITK_USE_TOF_PMDCAMBOARD)
  add_subdirectory(Testing)
ENDIF(MITK_USE_TOF_PMDO3 OR MITK_USE_TOF_PMDCAMCUBE OR MITK_USE_TOF_PMDCAMBOARD)

ENDIF(MITK_ENABLE_TOF_HARDWARE)
##CONFIGURE_FILE(mitkToFConfig.h.in ${PROJECT_BINARY_DIR}/mitkToFConfig.h @ONLY)
