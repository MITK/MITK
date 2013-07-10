SET(CPP_FILES
  mitkToFImageRecorder.cpp
  mitkToFImageRecorderFilter.cpp
  mitkToFImageWriter.cpp
  mitkToFNrrdImageWriter.cpp
  mitkToFImageCsvWriter.cpp
  mitkIToFDeviceFactory.cpp
  mitkAbstractToFDeviceFactory.cpp
  mitkToFHardwareActivator.cpp
  mitkToFCameraMITKPlayerDeviceFactory.cpp
  mitkToFImageGrabber.cpp
  mitkToFOpenCVImageGrabber.cpp
  mitkToFCameraDevice.cpp
  mitkToFCameraMITKPlayerController.cpp
  mitkToFCameraMITKPlayerDevice.cpp

)

set(RESOURCE_FILES
  CalibrationFiles/Default_Parameters.xml
  CalibrationFiles/Mesa-SR4000_Camera.xml
  CalibrationFiles/PMDCamCube3_camera.xml
  CalibrationFiles/Kinect_IR_camera.xml
  CalibrationFiles/PMDCamBoard_camera.xml
  CalibrationFiles/Kinect_RGB_camera.xml
  CalibrationFiles/PMDCamCube2_camera.xml
)

#IF(NOT MITK_USE_TOF_PMDCAMCUBE)
#IF(NOT MITK_USE_TOF_PMDCAMBOARD)
#IF(NOT MITK_USE_TOF_PMDO3)
#SET(CPP_FILES ${CPP_FILES}
#  mitkToFCameraPMDControllerStub.cpp
#)
#ENDIF(NOT MITK_USE_TOF_PMDO3)
#ENDIF(NOT MITK_USE_TOF_PMDCAMBOARD)
#ENDIF(NOT MITK_USE_TOF_PMDCAMCUBE)
