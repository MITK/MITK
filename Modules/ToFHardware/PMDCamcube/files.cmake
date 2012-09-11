IF(MITK_USE_TOF_PMDCAMCUBE)
SET(CPP_FILES
  mitkToFCameraPMDCamCubeDevice.cpp
  mitkToFCameraPMDCamCubeDeviceFactory.cpp
  mitkToFCameraPMDCamCubeController.cpp
  PMDCamCubeModuleActivator.cpp

  mitkToFCameraPMDController.cpp

  mitkToFCameraPMDRawDataCamCubeDevice.cpp
  mitkToFCameraPMDRawDataCamCubeDeviceFactory.cpp
  #the PMDRawDataCamCubeDeviceFactory need the
  mitkToFCameraPMDRawDataDevice.cpp
  #and the PMDRawDataDevice depends on
  mitkThreadedToFRawDataReconstruction.cpp

  mitkToFCameraPMDPlayerDevice.cpp
  mitkToFCameraPMDPlayerDeviceFactory.cpp
  mitkToFCameraPMDPlayerController.cpp

)
ENDIF(MITK_USE_TOF_PMDCAMCUBE)
