
SET(CPP_FILES
   mitkToFCameraPMDDevice.cpp

   mitkToFCameraPMDPlayerController.cpp
   mitkToFCameraPMDPlayerDevice.cpp
   mitkToFCameraPMDPlayerDeviceFactory.cpp
   #No Activator needed - this is not a MicroService


 #the PMDRawDataCamCubeDeviceFactory need the
   mitkToFCameraPMDRawDataDevice.cpp
 #and the PMDRawDataDevice depends on
   mitkThreadedToFRawDataReconstruction.cpp


  ##The ToFCameraPMDMITKPlayerDevice is depricated
     #mitkToFCameraPMDMITKPlayerController.cpp
     #mitkToFCameraPMDMITKPlayerDevice.cpp
)


##---------------------------------------------------Use of CamCube--------------------------------------------
IF(MITK_USE_TOF_PMDCAMCUBE)
SET(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDController.cpp

  mitkToFCameraPMDCamCubeController.cpp
  mitkToFCameraPMDCamCubeDevice.cpp
  mitkToFCameraPMDCamCubeDeviceFactory.cpp
  mitkPMDCamCubeModuleActivator.cpp

  mitkToFCameraPMDRawDataCamCubeDevice.cpp
  mitkToFCameraPMDRawDataCamCubeDeviceFactory.cpp

)
IF(WIN32)
  IF(CMAKE_CL_64)
   # SET(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerControllerStub.cpp)
  ELSE(CMAKE_CL_64)
    SET(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerController.cpp)
  ENDIF(CMAKE_CL_64)
ELSE(WIN32)
   # SET(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerControllerStub.cpp)
ENDIF(WIN32)
ELSE()
SET(CPP_FILES ${CPP_FILES}
  #mitkToFCameraPMDMITKPlayerControllerStub.cpp
  #mitkToFCameraPMDCamCubeControllerStub.cpp
  #mitkToFCameraPMDPlayerControllerStub.cpp
)
ENDIF(MITK_USE_TOF_PMDCAMCUBE)
##---------------------------------------------------------------------------------------------------------



#----------------------------------------------Use of CamBoard-----------------------------------------------
IF(MITK_USE_TOF_PMDCAMBOARD)
SET(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDController.cpp

  mitkToFCameraPMDCamBoardController.cpp
  mitkToFCameraPMDCamBoardDevice.cpp
  mitkToFCameraPMDCamBoardDeviceFactory.cpp
  mitkPMDCamBoardModuleActivator.cpp

   mitkToFCameraPMDRawDataCamBoardDevice.cpp
   mitkToFCameraPMDRawDataCamBoardDeviceFactory.cpp

)
ELSE()
SET(CPP_FILES ${CPP_FILES}
#mitkToFCameraPMDCamBoardControllerStub.cpp
)
ENDIF(MITK_USE_TOF_PMDCAMBOARD)
#---------------------------------------------------------------------------------------------------------



#------------------------------------------------Use of O3-----------------------------------------------
 IF(MITK_USE_TOF_PMDO3)
 SET(CPP_FILES ${CPP_FILES}
   mitkToFCameraPMDController.cpp

   mitkToFCameraPMDO3Device.cpp
   mitkToFCameraPMDO3DeviceFactory.cpp
   mitkToFCameraPMDO3Controller.cpp
   mitkPMDO3ModuleActivator.cpp


 )
 ELSE()
 SET(CPP_FILES ${CPP_FILES}
 #mitkToFCameraPMDO3ControllerStub.cpp
 )
 ENDIF(MITK_USE_TOF_PMDO3)


##------------------------------------------Stubs we do NOT want to use(if possible)---------------------------
#IF(NOT MITK_USE_TOF_PMDCAMCUBE)
#IF(NOT MITK_USE_TOF_PMDCAMBOARD)
#IF(NOT MITK_USE_TOF_PMDO3)
#SET(CPP_FILES ${CPP_FILES}
#  mitkToFCameraPMDControllerStub.cpp
#)
#ENDIF(NOT MITK_USE_TOF_PMDO3)
#ENDIF(NOT MITK_USE_TOF_PMDCAMBOARD)
#ENDIF(NOT MITK_USE_TOF_PMDCAMCUBE)
##---------------------------------------------------------------------------------------------------------
