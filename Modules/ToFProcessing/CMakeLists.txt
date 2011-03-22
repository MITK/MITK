#OPTION(MITK_USE_OpenCV "Use Intel's OpenCV library" OFF)

IF(MITK_USE_OpenCV)

MITK_CREATE_MODULE(mitkToFProcessing
    SUBPROJECTS MITK-ToF
    DEPENDS Mitk MitkExt
    PACKAGE_DEPENDS OpenCV
    PROVIDES mitkToFProcessing
  )
  
IF(BUILD_TESTING)

  ADD_SUBDIRECTORY(Testing)

ENDIF(BUILD_TESTING)

ELSE(MITK_USE_OpenCV)

    MESSAGE( "Not creating Module mitkToFProcessing. Set MITK_USE_OpenCV to ON." )

ENDIF(MITK_USE_OpenCV)


