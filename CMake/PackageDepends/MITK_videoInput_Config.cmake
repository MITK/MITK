SET( VIDEOINPUT_LIB "" CACHE FILEPATH "Search for the videoInput.lib file" )
SET( VIDEOINPUT_HEADER_DIR "" CACHE PATH "The path containing VideoInput.h" )

#message(STATUS "now in videoinput")

IF(CMAKE_CL_64)
  SET( STRMBASD_LIB "" CACHE FILEPATH "Search for the strmbasd.lib file. Provided by DirectShow, if not found, download Microsoft Platform SDK and go to the Samples_Multimedia_DirectShow_BaseClasses folder and compile the BaseClasses solution to generate the strmbasd.lib. Place it in the same folder as the videoInput.lib")
ENDIF(CMAKE_CL_64)    

SET( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:atlthunk" )
SET( VIDEOINPUT_LIBS ${VIDEOINPUT_LIB} ${STRMBASD_LIB} )
INCLUDE_DIRECTORIES( ${VIDEOINPUT_HEADER_DIR} )
ADD_DEFINITIONS(-DMITK_USE_VIDEOINPUT_LIB)
LIST(APPEND ALL_LIBRARIES ${VIDEOINPUT_LIBS})
LIST(APPEND ALL_INCLUDE_DIRECTORIES ${VIDEOINPUT_HEADER_DIR})

