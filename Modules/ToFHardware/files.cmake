set(CPP_FILES
  mitkToFImageGrabberCreator.cpp
  mitkToFImageGrabber.cpp
  mitkToFOpenCVImageGrabber.cpp
  mitkToFCameraDevice.cpp
  mitkToFCameraMITKPlayerController.cpp
  mitkToFCameraMITKPlayerDevice.cpp
  mitkToFCameraPMDDevice.cpp
  mitkToFCameraPMDRawDataDevice.cpp
  mitkToFCameraPMDPlayerDevice.cpp
  mitkToFCameraPMDMITKPlayerDevice.cpp
  mitkToFCameraPMDO3Device.cpp
  mitkToFCameraPMDCamCubeDevice.cpp
  mitkToFCameraPMDRawDataCamCubeDevice.cpp
  mitkToFCameraPMDCamBoardDevice.cpp
  mitkToFCameraPMDRawDataCamBoardDevice.cpp
  mitkToFCameraMESADevice.cpp
  mitkToFCameraMESASR4000Device.cpp
  mitkToFImageRecorder.cpp
  mitkToFImageRecorderFilter.cpp
  mitkToFImageWriter.cpp
  mitkToFNrrdImageWriter.cpp
  mitkToFPicImageWriter.cpp
  mitkToFImageCsvWriter.cpp
  mitkThreadedToFRawDataReconstruction.cpp
)

if(MITK_USE_TOF_PMDCAMCUBE)
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDMITKPlayerController.cpp
  mitkToFCameraPMDCamCubeController.cpp
  mitkToFCameraPMDController.cpp
)
if(WIN32)
  if(CMAKE_CL_64)
    set(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerControllerStub.cpp)
  else(CMAKE_CL_64)
    set(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerController.cpp)
  endif(CMAKE_CL_64)
else(WIN32)
    set(CPP_FILES ${CPP_FILES} mitkToFCameraPMDPlayerControllerStub.cpp)
endif(WIN32)
else()
set(CPP_FILES ${CPP_FILES}	
  mitkToFCameraPMDMITKPlayerControllerStub.cpp
  mitkToFCameraPMDCamCubeControllerStub.cpp
  mitkToFCameraPMDPlayerControllerStub.cpp
)
endif(MITK_USE_TOF_PMDCAMCUBE)

if(MITK_USE_TOF_PMDCAMBOARD)
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDCamBoardController.cpp
  mitkToFCameraPMDController.cpp
)
else()
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDCamBoardControllerStub.cpp
)
endif(MITK_USE_TOF_PMDCAMBOARD)


if(MITK_USE_TOF_PMDO3)
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDO3Controller.cpp
  mitkToFCameraPMDController.cpp
)
else()
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDO3ControllerStub.cpp
)
endif(MITK_USE_TOF_PMDO3)

if(NOT MITK_USE_TOF_PMDCAMCUBE)
if(NOT MITK_USE_TOF_PMDCAMBOARD)
if(NOT MITK_USE_TOF_PMDO3)
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraPMDControllerStub.cpp
)
endif(NOT MITK_USE_TOF_PMDO3)
endif(NOT MITK_USE_TOF_PMDCAMBOARD)
endif(NOT MITK_USE_TOF_PMDCAMCUBE)

if(MITK_USE_TOF_MESASR4000)
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraMESASR4000Controller.cpp
  mitkToFCameraMESAController.cpp
)
else()
set(CPP_FILES ${CPP_FILES}
  mitkToFCameraMESAControllerStub.cpp
  mitkToFCameraMESASR4000ControllerStub.cpp
)
endif(MITK_USE_TOF_MESASR4000)


