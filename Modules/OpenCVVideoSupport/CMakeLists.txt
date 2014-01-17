if(MITK_USE_OpenCV)

  set(dependencies ITK>ITKVideoBridgeOpenCV OpenCV)
  if(MITK_USE_videoInput)
    set(dependencies ${dependencies} videoInput)
  endif(MITK_USE_videoInput)

  MITK_CREATE_MODULE(mitkOpenCVVideoSupport
    INCLUDE_DIRS Commands
    DEPENDS MitkAlgorithmsExt
    PACKAGE_DEPENDS ${dependencies}
    ADDITIONAL_LIBS ${OPENCVVIDEOSUPPORT_ADDITIONAL_LIBS}
    EXPORT_DEFINE MITK_OPENCVVIDEOSUPPORT_EXPORT
  )

  if(MITK_USE_QT)
    add_subdirectory(UI)
  endif(MITK_USE_QT)

  if(BUILD_TESTING)
    add_subdirectory(Testing)
  endif(BUILD_TESTING)

endif(MITK_USE_OpenCV)
