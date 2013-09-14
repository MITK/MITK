if(MITK_USE_TOF_MESASR4000)
SET(CPP_FILES
  mitkToFCameraMESASR4000Device.cpp
  mitkToFCameraMESASR4000DeviceFactory.cpp
  mitkToDCameraMESASR4000ModuleActivator.cpp
  mitkToFCameraMESASR4000Controller.cpp
  mitkToFCameraMESADevice.cpp
  mitkToFCameraMESAController.cpp
)
set(RESOURCE_FILES
  CalibrationFiles/Mesa-SR4000_Camera.xml
)

ENDIF(MITK_USE_TOF_MESASR4000)
