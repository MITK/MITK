
set(CPP_FILES
    mitkMovieGeneratorOpenCV.cpp
    mitkUndistortCameraImage.cpp
    mitkOpenCVVideoSource.cpp
    mitkOpenCVToMitkImageFilter.cpp
    mitkImageToOpenCVImageFilter.cpp
)

if(MITK_USE_videoInput)
  set(CPP_FILES ${CPP_FILES}
    mitkVideoInputSource.cpp
  )
endif(MITK_USE_videoInput)