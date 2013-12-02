IF(MITK_USE_TOF_KINECT2)
SET(CPP_FILES
  mitkKinect2Device.cpp
  mitkKinect2DeviceFactory.cpp
  mitkKinect2Controller.cpp
  mitkKinect2Activator.cpp
)
set(RESOURCE_FILES
  #CalibrationFiles/Kinect_IR_camera.xml
  #CalibrationFiles/Kinect_RGB_camera.xml
)

ENDIF(MITK_USE_TOF_KINECT2)
