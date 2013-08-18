IF(MITK_USE_TOF_KINECT)
SET(CPP_FILES
  mitkKinectDevice.cpp
  mitkKinectDeviceFactory.cpp
  mitkKinectController.cpp
  mitkKinectActivator.cpp
)
set(RESOURCE_FILES
  CalibrationFiles/Kinect_IR_camera.xml
  CalibrationFiles/Kinect_RGB_camera.xml
)

ENDIF(MITK_USE_TOF_KINECT)
