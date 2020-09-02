#Begin PMD Module
OPTION(MITK_USE_TOF_PMDCAMCUBE "Enable support for Cam Cube camera" OFF)

IF(WIN32)
  IF(_PLATFORM_STRING MATCHES "W32")
    OPTION(MITK_USE_TOF_PMDCAMBOARD "Enable support for PMD camboard" OFF)
  ENDIF(_PLATFORM_STRING MATCHES "W32")

ElSE(WIN32)
    OPTION(MITK_USE_TOF_PMDCAMBOARD "Enable support for PMD camboard" OFF)
ENDIF(WIN32)

IF(WIN32)
  IF(_PLATFORM_STRING MATCHES "W32")
      OPTION(MITK_USE_TOF_PMDO3 "Enable support for PMD O3" OFF)
  ENDIF(_PLATFORM_STRING MATCHES "W32")
ENDIF(WIN32)

SET(MITK_USE_PMD OFF)
IF(MITK_USE_TOF_PMDO3 OR MITK_USE_TOF_PMDCAMCUBE OR MITK_USE_TOF_PMDCAMBOARD)
  SET(MITK_USE_PMD ON)
ENDIF(MITK_USE_TOF_PMDO3 OR MITK_USE_TOF_PMDCAMCUBE OR MITK_USE_TOF_PMDCAMBOARD)


#Creating Basics for all PMD devices
IF(MITK_USE_PMD)
  #-----Setting generall Path-----
  FIND_LIBRARY(MITK_PMD_LIB pmdaccess2 DOC "PMD access library.")
  GET_FILENAME_COMPONENT(MITK_PMD_SDK_DIR ${MITK_PMD_LIB} PATH)
  SET(MITK_PMD_SDK_DIR ${MITK_PMD_SDK_DIR}/.. CACHE INTERNAL "PMD SDK directory")
  FIND_PATH(MITK_PMD_INCLUDE_DIR pmdsdk2.h ${MITK_PMD_SDK_DIR}/include DOC  "Include directory of PMD-SDK.")

    Set(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_PMD_SDK_DIR}/include)
    Set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_PMD_LIB})

  #-----Creating the PMD-Module itself-----
  MITK_CREATE_MODULE(
    INCLUDE_DIRS ${MITK_BINARY_DIR}
    INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
    DEPENDS MitkToFHardware
    ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
    AUTOLOAD_WITH MitkToFHardware
  )
  Message("PMD Module generated")
  add_subdirectory(Testing)
ENDIF(MITK_USE_PMD)

#Only for CamCube
IF(MITK_USE_TOF_PMDCAMCUBE)
#  message(${MITK_BINARY_DIR})
#  message(${MITK_PMD_SDK_DIR})
#  message(${_PLATFORM_STRING})
  configure_file(${MITK_PMD_SDK_DIR}/plugins/camcube3.${_PLATFORM_STRING}.pap ${MITK_BINARY_DIR}/bin/camcube3.${_PLATFORM_STRING}.pap COPYONLY)
  configure_file(${MITK_PMD_SDK_DIR}/plugins/camcubeproc.${_PLATFORM_STRING}.ppp ${MITK_BINARY_DIR}/bin/camcubeproc.${_PLATFORM_STRING}.ppp COPYONLY)
  IF(_PLATFORM_STRING MATCHES "W32")
    configure_file(${MITK_PMD_SDK_DIR}/plugins/pmdfile.${_PLATFORM_STRING}.pcp ${MITK_BINARY_DIR}/bin/pmdfile.${_PLATFORM_STRING}.pap COPYONLY)
  ENDIF()

  SET(MITK_TOF_PMD_CAMCUBE_SOURCE camcube3.${_PLATFORM_STRING}.pap)
  SET(MITK_TOF_PMD_CAMCUBE_PROC camcubeproc.${_PLATFORM_STRING}.ppp)
  IF(_PLATFORM_STRING MATCHES "W32")
    SET(MITK_TOF_PMD_FILE_SOURCE pmdfile.${_PLATFORM_STRING}.pcp)
  ENDIF()
  SET(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD CamCube 2.0/3.0,PMD CamCubeRaw 2.0/3.0,PMD Player,PMD Raw Data Player)

Message("PMDCamCube activated")
ENDIF(MITK_USE_TOF_PMDCAMCUBE)

#-------------------------------Begin CamBoard hardware-------------------------------------------
IF(MITK_USE_TOF_PMDCAMBOARD)
  IF(_PLATFORM_STRING MATCHES "W32")
    configure_file(${MITK_PMD_SDK_DIR}/plugins/camboard.${_PLATFORM_STRING}.pap ${MITK_BINARY_DIR}/bin/camboard.${_PLATFORM_STRING}.pap COPYONLY)
    configure_file(${MITK_PMD_SDK_DIR}/plugins/camboardproc.${_PLATFORM_STRING}.ppp ${MITK_BINARY_DIR}/bin/camboardproc.${_PLATFORM_STRING}.ppp COPYONLY)
  ENDIF()

   SET(MITK_TOF_PMD_CAMBOARD_SOURCE camboard.${_PLATFORM_STRING}.pap)
   SET(MITK_TOF_PMD_CAMBOARD_PROC camboardproc.${_PLATFORM_STRING}.ppp)
   SET(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD CamBoard,PMD CamBoardRaw)
#  FILE(GLOB MITK_TOF_PMD_CAMBOARD_SOURCE camboard.${_PLATFORM_STRING}.pap)
#  FILE(GLOB MITK_TOF_PMD_CAMBOARD_PROC camboardproc.${_PLATFORM_STRING}.ppp)
#  FILE(GLOB MITK_TOF_PMD_FILE_SOURCE ${MITK_PMD_SDK_DIR}/plugins/pmdfile.${_PLATFORM_STRING}.pcp)

Message("PMDCamBoard activated")
ENDIF(MITK_USE_TOF_PMDCAMBOARD)

#----------------------------------------Begin PMD O3 hardware--------------------------------------------

# only if PMD O3 is enabled
IF(WIN32 AND MITK_USE_TOF_PMDO3)
Message("PMDO3 is activated")

   configure_file(${MITK_PMD_SDK_DIR}/plugins/o3d.${_PLATFORM_STRING}.pcp ${MITK_BINARY_DIR}/bin/o3d.${_PLATFORM_STRING}.pcp COPYONLY)
   configure_file(${MITK_PMD_SDK_DIR}/plugins/o3d.${_PLATFORM_STRING}.pcp ${MITK_BINARY_DIR}/bin/o3d.${_PLATFORM_STRING}.pcp COPYONLY)

   SET(MITK_TOF_PMD_O3D_SOURCE o3d.${_PLATFORM_STRING}.pcp)
   SET(MITK_TOF_PMD_O3D_PROC o3d.${_PLATFORM_STRING}.pcp)
   SET(MITK_TOF_AVAILABLE_CAMERAS ${MITK_TOF_AVAILABLE_CAMERAS},PMD O3D)
 ENDIF()

 #Generate the mitkToFPMDConfig.h file which is used internally
CONFIGURE_FILE(mitkToFPMDConfig.h.in ${PROJECT_BINARY_DIR}/mitkToFPMDConfig.h @ONLY)
