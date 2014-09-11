IF(MITK_USE_TOF_KINECTV2)
SET(CPP_FILES
  mitkKinectV2Device.cpp
  mitkKinectV2DeviceFactory.cpp
  mitkKinectV2Controller.cpp
  mitkKinectV2Activator.cpp
)
set(RESOURCE_FILES
  #Todo: Replace me one we have actual calibration files
  CalibrationFiles/KinectV2_IR_camera.xml
  CalibrationFiles/KinectV2_RGB_camera.xml
)

ENDIF(MITK_USE_TOF_KINECTV2)
