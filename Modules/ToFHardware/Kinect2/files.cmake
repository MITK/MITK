IF(MITK_USE_TOF_KINECT2)
SET(CPP_FILES
  mitkKinect2Device.cpp
  mitkKinect2DeviceFactory.cpp
  mitkKinect2Controller.cpp
  mitkKinect2Activator.cpp
)
SET(H_FILES
  stdafx.h
)
set(RESOURCE_FILES
  #Todo: Replace me one we have actual calibration files
  CalibrationFiles/Kinect_RGB_camera.xml
)

ENDIF(MITK_USE_TOF_KINECT2)
