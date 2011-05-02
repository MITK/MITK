
SET(CPP_FILES
    mitkMovieGeneratorOpenCV.cpp
    mitkUndistortCameraImage.cpp
    mitkOpenCVVideoSource.cpp
    mitkOpenCVToMitkImageFilter.cpp
    mitkImageToOpenCVImageFilter.cpp
)

IF(MITK_USE_videoInput)
  SET(CPP_FILES ${CPP_FILES}
    mitkVideoInputSource.cpp
  )
ENDIF(MITK_USE_videoInput)