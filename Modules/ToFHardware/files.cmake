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

#IF(NOT MITK_USE_TOF_PMDCAMCUBE)
#IF(NOT MITK_USE_TOF_PMDCAMBOARD)
#IF(NOT MITK_USE_TOF_PMDO3)
#SET(CPP_FILES ${CPP_FILES}
#  mitkToFCameraPMDControllerStub.cpp
#)
#ENDIF(NOT MITK_USE_TOF_PMDO3)
#ENDIF(NOT MITK_USE_TOF_PMDCAMBOARD)
#ENDIF(NOT MITK_USE_TOF_PMDCAMCUBE)
