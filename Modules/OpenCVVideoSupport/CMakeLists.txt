OPTION(MITK_USE_OPEN_CV "Use Intel's OpenCV library" OFF)

IF(MITK_USE_OPEN_CV)

MESSAGE( STATUS "Creating Module mitkOpenCVVideoSupport" )

  MITK_CREATE_MODULE(mitkOpenCVVideoSupport
    DEPENDS Mitk MitkExt
    PACKAGE_DEPENDS OpenCV
    PROVIDES mitkOpenCVVideoSupport
    EXPORT_DEFINE MITK_OPENCVVIDEOSUPPORT_EXPORT
  )
  
  IF(MITK_USE_QT)
	ADD_SUBDIRECTORY(UI)
  ENDIF(MITK_USE_QT)  
    
  IF(BUILD_TESTING)
	ADD_SUBDIRECTORY(Testing)
  ENDIF(BUILD_TESTING)  
  
ELSE(MITK_USE_OPEN_CV)

	MESSAGE( STATUS "Not creating Module mitkOpenCVVideoSupport" )
	
ENDIF(MITK_USE_OPEN_CV)

