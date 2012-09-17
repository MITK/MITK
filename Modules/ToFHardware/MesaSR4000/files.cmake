if(MITK_USE_TOF_MESASR4000)
SET(CPP_FILES

  mitkToFCameraMESAController.cpp
  mitkToFCameraMESADevice.cpp
  mitkToFCameraMESASR4000Controller.cpp
  mitkToFCameraMESASR4000Device.cpp
  mitkToFCameraMESASR4000DeviceFactory.cpp
  #mitkToFCameraMESAControllerStub.cpp
  #mitkToFCameraMESASR4000ControllerStub.cpp
)

ENDIF(MITK_USE_TOF_MESASR4000)
