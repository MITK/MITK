OPTION(MITK_USE_OPEN_CV "Use Intel's OpenCV library" OFF)

IF(MITK_USE_OPEN_CV)

#MESSAGE( STATUS "Creating Module mitkOpenCVVideoSupport" )

# check if the operating system is windows. This command does not check wether the system is running on 32 or 64 bit
IF(WIN32)
  OPTION(MITK_USE_VIDEOINPUT_LIB "Use videoInput (DirectShow wrapper) library" OFF)

  IF(MITK_USE_VIDEOINPUT_LIB)
    SET( VIDEOINPUT_LIB "" CACHE FILEPATH "Search for the videoInput.lib file" )
    SET( VIDEOINPUT_HEADER_DIR "" CACHE PATH "The path containing VideoInput.h" )
    IF(CMAKE_CL_64)
      SET( STRMBASD_LIB "" CACHE FILEPATH "Search for the strmbasd.lib file. Provided by DirectShow, if not found, download Microsoft Platform SDK and go to the Samples_Multimedia_DirectShow_BaseClasses folder and compile the BaseClasses solution to generate the strmbasd.lib. Place it in the same folder as the videoInput.lib")
    ENDIF(CMAKE_CL_64)    
    SET( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:atlthunk" )
    SET( OPENCVVIDEOSUPPORT_ADDITIONAL_LIBS ${VIDEOINPUT_LIB} ${STRMBASD_LIB} )
    INCLUDE_DIRECTORIES( ${VIDEOINPUT_HEADER_DIR} )
    ADD_DEFINITIONS(-DMITK_USE_VIDEOINPUT_LIB)
  ENDIF(MITK_USE_VIDEOINPUT_LIB)

 ELSE(WIN32)
  SET(MITK_USE_VIDEOINPUT_LIB OFF)
ENDIF(WIN32)

  MITK_CREATE_MODULE(mitkOpenCVVideoSupport
    DEPENDS Mitk MitkExt
    PACKAGE_DEPENDS OpenCV
    PROVIDES mitkOpenCVVideoSupport
    ADDITIONAL_LIBS ${OPENCVVIDEOSUPPORT_ADDITIONAL_LIBS}
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

