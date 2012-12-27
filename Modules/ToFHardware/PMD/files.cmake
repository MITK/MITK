IF(MITK_USE_PMD)
SET(CPP_FILES

   mitkToFCameraPMDPlayerController.cpp
   mitkToFCameraPMDPlayerDevice.cpp
   mitkToFCameraPMDPlayerDeviceFactory.cpp
   mitkToFCameraPMDDevice.cpp
 #the PMDRawDataCamCubeDeviceFactory need the
   mitkToFCameraPMDRawDataDevice.cpp
 #and the PMDRawDataDevice depends on
   mitkThreadedToFRawDataReconstruction.cpp
   mitkPMDModuleActivator.cpp
   mitkToFCameraPMDController.cpp #wenn nichts geht, dann wieder nach unten
)
ENDIF(MITK_USE_PMD)

##---------------------------------------------------Use of CamCube--------------------------------------------
IF(MITK_USE_TOF_PMDCAMCUBE)
SET(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDController.cpp
  mitkToFCameraPMDCamCubeController.cpp
  mitkToFCameraPMDCamCubeDevice.cpp
  mitkToFCameraPMDCamCubeDeviceFactory.cpp
  mitkToFCameraPMDRawDataCamCubeDevice.cpp
  mitkToFCameraPMDRawDataCamCubeDeviceFactory.cpp
)
IF(WIN32)
#mitkToFCameraPMDPlayerController only works for 32 bit
  IF(NOT CMAKE_CL_64)
    SET(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerController.cpp mitkToFCameraPMDPlayerDevice.cpp mitkToFCameraPMDPlayerDeviceFactory.cpp)
  ENDIF(NOT CMAKE_CL_64)
ELSE(WIN32)
   # SET(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerControllerStub.cpp)
ENDIF(WIN32)
ENDIF(MITK_USE_TOF_PMDCAMCUBE)

#----------------------------------------------Use of CamBoard-----------------------------------------------
IF(MITK_USE_TOF_PMDCAMBOARD)
SET(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDCamBoardController.cpp
  mitkToFCameraPMDCamBoardDevice.cpp
  mitkToFCameraPMDCamBoardDeviceFactory.cpp
  mitkToFCameraPMDRawDataCamBoardDevice.cpp
  mitkToFCameraPMDRawDataCamBoardDeviceFactory.cpp
)
ENDIF(MITK_USE_TOF_PMDCAMBOARD)
#---------------------------------------------------------------------------------------------------------

#------------------------------------------------Use of O3-----------------------------------------------
 IF(MITK_USE_TOF_PMDO3)
 SET(CPP_FILES ${CPP_FILES}
   mitkToFCameraPMDO3Device.cpp
   mitkToFCameraPMDO3DeviceFactory.cpp
   mitkToFCameraPMDO3Controller.cpp
 )
 ENDIF(MITK_USE_TOF_PMDO3)
